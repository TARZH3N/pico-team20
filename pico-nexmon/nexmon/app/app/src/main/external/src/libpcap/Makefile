all: libs/armeabi/libpcap.a
	cp local/armeabi/libpcap.a ../../../libs/armeabi

libs/armeabi/libpcap.a: Android.mk
	$(NDK_ROOT)/ndk-build NDK_APPLICATION_MK=`pwd`/Application.mk NDK_APP_OUT=. TARGET_PLATFORM=android-21

clean:
	rm -Rf libs
	rm -Rf local
