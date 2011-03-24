/* * This file is part of meego-im-framework *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */
#include "mpassthruwindow.h"
#include "mimapplication.h"
#include "mimremotewindow.h"

#include <QDebug>
#include <QGraphicsView>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#ifndef M_IM_DISABLE_TRANSLUCENCY
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/shape.h>
#endif

MPassThruWindow::MPassThruWindow(QWidget *p)
    : QWidget(p),
      remoteWindow(0)
{
    setWindowTitle("MInputMethod");
    setFocusPolicy(Qt::NoFocus);

    if (mApp && mApp->selfComposited()) {
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_NoSystemBackground);
    } else {
        setAttribute(Qt::WA_TranslucentBackground);
    }

    Qt::WindowFlags windowFlags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;

    if (mApp && mApp->bypassWMHint()) {
        windowFlags |= Qt::X11BypassWindowManagerHint;
    }

    setWindowFlags(windowFlags);

    // We do not want input focus for that window.
    setAttribute(Qt::WA_X11DoNotAcceptFocus);

    QObject::connect(mApp, SIGNAL(remoteWindowChanged(MImRemoteWindow *)),
                     this, SLOT(setRemoteWindow(MImRemoteWindow *)));
}

MPassThruWindow::~MPassThruWindow()
{
}

void MPassThruWindow::inputPassthrough(const QRegion &region)
{
#ifndef M_IM_DISABLE_TRANSLUCENCY
    Display *dpy = QX11Info::display();
#endif

    qDebug() << __PRETTY_FUNCTION__ << "QWidget::effectiveWinId(): " << effectiveWinId();

    // Set _NET_WM_WINDOW_TYPE to _NET_WM_WINDOW_TYPE_INPUT
    static Atom input = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_INPUT", False);
    static Atom window_type = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False);
    XChangeProperty(QX11Info::display(), effectiveWinId(), window_type, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *) &input, 1);

    qDebug() << __PRETTY_FUNCTION__ << region << "geometry=" << geometry();
    QVector<QRect> regionRects(region.rects());
    const int size = regionRects.size();

    if (size) {
        XRectangle * const rects = (XRectangle*)malloc(sizeof(XRectangle)*(size));
        if (!rects) {
            return;
        }

        quint32 customRegion[size * 4]; // custom region is pack of x, y, w, h
        XRectangle *rect = rects;
        for (int i = 0; i < size; ++i, ++rect) {
            rect->x = regionRects.at(i).x();
            rect->y = regionRects.at(i).y();
            rect->width = regionRects.at(i).width();
            rect->height = regionRects.at(i).height();
            customRegion[i * 4 + 0] = rect->x;
            customRegion[i * 4 + 1] = rect->y;
            customRegion[i * 4 + 2] = rect->width;
            customRegion[i * 4 + 3] = rect->height;
        }

        const XserverRegion shapeRegion = XFixesCreateRegion(dpy, rects, size);
        XFixesSetWindowShapeRegion(dpy, effectiveWinId(), ShapeBounding, 0, 0, 0);
        XFixesSetWindowShapeRegion(dpy, effectiveWinId(), ShapeInput, 0, 0, shapeRegion);

        XFixesDestroyRegion(dpy, shapeRegion);

        XChangeProperty(dpy, effectiveWinId(), XInternAtom(dpy, "_MEEGOTOUCH_CUSTOM_REGION", False), XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char *) customRegion, size * 4);

        free(rects);
        XSync(dpy, False);
    }

    // selective compositing
    if (region.isEmpty()) {
        if (mApp && mApp->selfComposited() && remoteWindow) {
            remoteWindow->unredirect();
        }

        hide();
    } else {
        if (mApp && mApp->selfComposited() && remoteWindow) {
            remoteWindow->redirect();
        }

        // Do not call multiple times showFullScreen() because it could
        // break focus from the activateWindow() call. See QTBUG-18130
        if (!isVisible()) {
            showFullScreen();

            // If bypassing window hint, also do raise to ensure visibility:
            if (mApp && mApp->bypassWMHint()) {
                raise();
            }
        }
    }
}

void MPassThruWindow::setRemoteWindow(MImRemoteWindow *newWindow)
{
    remoteWindow = newWindow;

    if (!newWindow)
        inputPassthrough();
}

void MPassThruWindow::updateFromRemoteWindow(const QRegion &region)
{
    const QRectF br(region.boundingRect());
    QList<QRectF> rects;
    rects.append(br);

    foreach (QObject *obj, children()) {
        if  (QGraphicsView *v = qobject_cast<QGraphicsView *>(obj)) {
            v->invalidateScene(br, QGraphicsScene::BackgroundLayer);
            v->updateScene(rects);
            v->update(region);
        } else
        if (QWidget *w = qobject_cast<QWidget *>(obj)) {
            w->update(region);
        }
    }
}
