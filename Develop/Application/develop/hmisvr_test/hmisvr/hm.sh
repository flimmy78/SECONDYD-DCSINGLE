#!  /bin/sh
make distclean
/usr/local/Trolltech/Qte-4.8.4-arm/bin/qmake
#/usr/local/Trolltech/QtEmbedded-4.8.4-arm/bin/qmake
make -j8
#cp hmisvr $NEWYD_DCSINGLE_AY_DC_APP_RUNTIME_DIR

