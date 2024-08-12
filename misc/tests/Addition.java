
class Addition {
    static int x = 3;
    static int y = 5;

    private static int addLiterals() {
        return 3 + 5;
    }

    private static int addFields() {
        return x + y;
    }

    private static int addArguments(int x, int y) {
        return x + y;
    }

    public static void main(String[] args) {
        int a = addLiterals();
        int b = addArguments(3, 5);
        int c = addFields();
    }
}
