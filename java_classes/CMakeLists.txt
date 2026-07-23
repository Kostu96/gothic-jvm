find_package(Java REQUIRED)

set(JAVA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/com/kostu96/gjvm/io/ResourceInputStream.java
    ${CMAKE_CURRENT_SOURCE_DIR}/com/kostu96/gjvm/media/BasePlayer.java

    ${CMAKE_CURRENT_SOURCE_DIR}/com/nokia/mid/ui/FullCanvas.java

    ${CMAKE_CURRENT_SOURCE_DIR}/java/io/ByteArrayInputStream.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/io/IOException.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/Class.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/ClassNotFoundException.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/Exception.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/Integer.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/NullPointerException.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/Object.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/Runnable.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/Runtime.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/RuntimeException.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/String.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/StringBuffer.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/System.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/Thread.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/Throwable.java

    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/Canvas.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/Command.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/CommandListener.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/Display.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/Displayable.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/Font.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/Graphics.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/Image.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/media/Control.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/media/Controllable.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/media/Manager.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/media/MediaException.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/media/Player.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/media/PlayerListener.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/midlet/MIDlet.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/midlet/MIDletStateChangeException.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/rms/InvalidRecordIDException.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/rms/RecordStore.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/rms/RecordStoreException.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/rms/RecordStoreFullException.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/rms/RecordStoreNotFoundException.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/rms/RecordStoreNotOpenException.java
)

set(CLASS_OUTPUT_DIR ${CMAKE_BINARY_DIR}/java_classes)

add_custom_command(
    OUTPUT ${CLASS_OUTPUT_DIR}/javac.stamp
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CLASS_OUTPUT_DIR}
    COMMAND ${Java_JAVAC_EXECUTABLE}
            -d ${CLASS_OUTPUT_DIR}
            -source 1.3
            -target 1.1
            ${JAVA_SOURCES}
    COMMAND ${CMAKE_COMMAND} -E touch ${CLASS_OUTPUT_DIR}/javac.stamp
    DEPENDS ${JAVA_SOURCES}
    COMMENT "Building java classes."
)

add_custom_target(java_classes ALL
    DEPENDS ${CLASS_OUTPUT_DIR}/javac.stamp
)
