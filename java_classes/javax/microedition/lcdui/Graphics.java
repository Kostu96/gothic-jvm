package javax.microedition.lcdui;

public class Graphics {

    public static final int HCENTER = 1;
	public static final int VCENTER = 2;
	public static final int LEFT = 4;
	public static final int RIGHT = 8;
	public static final int TOP = 16;
	public static final int BOTTOM = 32;
	public static final int BASELINE = 64;

    public void drawImage(Image image, int x, int y, int anchor) {
		if ((anchor & RIGHT) != 0) {
			x -= image.getWidth();
		} else if ((anchor & HCENTER) != 0) {
			x -= image.getWidth() / 2;
		}

		if ((anchor & BOTTOM) != 0) {
			y -= image.getHeight();
		} else if ((anchor & VCENTER) != 0) {
			y -= image.getHeight() / 2;
		}

        drawImageNative(image, x, y);
    }

    public void drawRect(int x, int y, int width, int height) {
		if (width < 0 || height < 0) return;

		drawRectNative(x, y, width, height);
	}

    public native void drawRegion(Image image, int x_src, int y_src, int width, int height,
						          int transform, int x_dst, int y_dst, int anchor);

    public void drawString(String str, int x, int y, int anchor) {
        if (str == null) {
            throw new NullPointerException();
        }
 
        if (anchor == 0) {
	        anchor = TOP | LEFT;
	    } else if (!checkAnchor(anchor, VCENTER)) {
            throw new IllegalArgumentException();
        }

        //x += transX; TODO(Kostu): commented until translation is needed
        //y += transY;

        if ((anchor & LEFT) == 0) {
            int strWidth = font.stringWidth(str);
            if ((anchor & RIGHT) != 0) {
                x -= strWidth;
            } else if ((anchor & HCENTER) != 0) {
                x -= (strWidth / 2);
            }
        }

        if ((anchor & BASELINE) == 0) {
            if ((anchor & TOP) != 0) {
                y += font.getBaselinePosition();
            } else if ((anchor & BOTTOM) != 0) {
                y -= font.getHeight() - 
                     font.getBaselinePosition();
            }
        }

        drawStringNative(str, x, y);
    }

    public native void fillRect(int x, int y, int width, int height);

    public void setClip(int x, int y, int width, int height) {}

    public void setColor(int rgb) {
        color = 0xFF000000 | (rgb & 0x00FFFFFF);
    }

    public void setFont(Font font) {
        this.font = font == null ? Font.getDefaultFont() : font;
    }

    Graphics() {
        init();
    }

    Graphics(Image image) {
        init(image);
    }

    private boolean checkAnchor(int anchor, int illegal_vpos) {
        /* optimize for most frequent case */
        if (anchor == (TOP|LEFT) || anchor == 0) {
            return true;
        }

        boolean result = (anchor > 0)  && (anchor < (BASELINE << 1)) &&
	                     ((anchor & illegal_vpos) == 0);

        if (result) {
            int n = anchor & (TOP | BOTTOM | BASELINE | VCENTER);
            /* exactly one bit set */
            result = (n != 0) && ((n & (n - 1)) == 0); 
        }

        if (result) {
            int n = anchor & (LEFT | RIGHT | HCENTER);
            /* exactly one bit set */
            result = (n != 0) && ((n & (n - 1)) == 0);
        }
	
        return result;
    }

    private native void drawImageNative(Image image, int x, int y);
    private native void drawStringNative(String str, int x, int y);
    private native void drawRectNative(int x, int y, int width, int height);
    private native void init();
    private native void init(Image image);

    private int color = -1;
    private Font font;
}
