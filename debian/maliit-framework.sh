# Exports GTK_IM_MODULE and QT_IM_MODULE under Unity 8

case $XDG_SESSION_DESKTOP in
  ubuntu-touch|unity8*)
    export GTK_IM_MODULE=Maliit
    export QT_IM_MODULE=maliitphablet
    ;;
esac
