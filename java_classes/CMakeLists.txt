find_package(Java REQUIRED)

set(JAVA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/com/kostu96/gjvm/ResourceInputStream.java

    ${CMAKE_CURRENT_SOURCE_DIR}/com/nokia/mid/ui/FullCanvas.java

    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/Class.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/String.java
    ${CMAKE_CURRENT_SOURCE_DIR}/java/lang/System.java

    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/Canvas.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/Font.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/Graphics.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/lcdui/Image.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/midlet/MIDlet.java
    ${CMAKE_CURRENT_SOURCE_DIR}/javax/microedition/midlet/MIDletStateChangeException.java
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
)

add_custom_target(java_classes ALL
    DEPENDS ${CLASS_OUTPUT_DIR}/javac.stamp
)
