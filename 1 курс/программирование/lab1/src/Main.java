import static java.lang.Math.*;

public class Main {
    public static void main(String[] args) {
        int[] e = new int[(25 - 3) / 2 + 1];
        for (int i = 0; i < e.length; i++) {
            e[i] = i * 2 + 3;
        }

        double[] x = new double[15];
        for (int i = 0; i < x.length; i++) {
            x[i] = -6.0 + random() * (8.0 + 6.0);
        }

        double[][] f = new double[12][15];
        for (int i = 0; i < f.length; i++) {
            for (int j = 0; j < f[i].length; j++) {
                f[i][j] = calculateValue(e[i], x[j]);
            }
        }
        printAnswer(f);
    }
    static double calculateValue(int eValue, double xValue) {
        if (eValue == 19) {
            return pow(atan(0.2 * ((xValue + 1) / 14)), 0.25/(asin(pow(E, -abs(xValue))) - 1));
        } else if (isInclude(eValue)) {
            return cos(cos(cbrt(xValue)));
        } else {
            return pow(0.25 - pow(cbrt(pow(xValue, (xValue - 3.0/4.0))), 3 + cos(cbrt(xValue))), asin(pow(E, cbrt (-pow((4 / abs(xValue)), xValue)))));
        }
    }
    static boolean isInclude(int eValue) {
        return eValue == 5 || eValue == 9 || eValue == 11 || eValue == 13 || eValue == 17 || eValue == 23;
    }
    static void printAnswer(double[][] f) {
        for (int i = 0; i < f.length; i++) {
            for (int j = 0; j < f[i].length; j++) {
                System.out.printf("%.4f ", f[i][j]);
            }
            System.out.println();
        }
    }
}
