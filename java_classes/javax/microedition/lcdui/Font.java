package javax.microedition.lcdui;

public class Font {
	public static final int FACE_SYSTEM = 0;
    public static final int FACE_MONOSPACE = 32;
	public static final int FACE_PROPORTIONAL = 64;

	public static final int SIZE_MEDIUM = 0;
	public static final int SIZE_SMALL = 8;
	public static final int SIZE_LARGE = 16;

	public static final int STYLE_PLAIN = 0;
	public static final int STYLE_BOLD = 1;
	public static final int STYLE_ITALIC = 2;
	public static final int STYLE_UNDERLINED = 4;

	public static final int FONT_STATIC_TEXT = 0;
	public static final int FONT_INPUT_TEXT = 1;

    public static Font getDefaultFont() {
        if (DEFAULT_FONT == null)
            DEFAULT_FONT = new Font();
        return DEFAULT_FONT;
    }

    public static Font getFont(int face, int style, int size) {
        if (DEFAULT_FONT == null)
            DEFAULT_FONT = new Font();
        return DEFAULT_FONT;
    }

    public int getHeight() {
        return 16;
    }

    public int stringWidth(String str) {
        return str.length() * 8;
    }

    private Font() {
        init();
    }

    private native void init();

    private static Font DEFAULT_FONT;
}
