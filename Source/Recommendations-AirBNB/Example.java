import java.util.*;

class Rating{
    private int item;
    private int rating;

    public Rating(int item, int rating){
        this.item = item;
        this.rating = rating;
    }

    public int getItem() {
        return item;
    }

    public void setItem(int item) {
        this.item = item;
    }

    public int getRating() {
        return rating;
    }

    public void setRating(int rating) {
        this.rating = rating;
    }
}

public class Example {

    public native void initLSH(int itemsNum);
    public native void addLSHPoint(int[] items, int[] ratings);
    public native int[] getRecommendedItems(int userIndex, int top);

    private static int ITEMS = 100;

    private static List<List<Rating>> UsersRatings = new ArrayList<>();

    static {
        UsersRatings.add(Arrays.asList(
                new Rating(14, 2),
                new Rating(26, 5),
                new Rating(28, 1)
        ));
        UsersRatings.add(Arrays.asList(
                new Rating(4, 2),
                new Rating(36, 5),
                new Rating(68, 1)
        ));
        UsersRatings.add(Arrays.asList(
                new Rating(24, 2),
                new Rating(26, 5),
                new Rating(78, 1)
        ));
    }

    public static void main(String[] args) {
        System.loadLibrary("Example");
        Example example = new Example();
        example.initLSH(ITEMS);
        for( List<Rating> userRatings: UsersRatings){
            int items[] = new int[userRatings.size()];
            int ratings[] = new int[userRatings.size()];
            for (int i = 0; i < userRatings.size(); i++) {
                items[i] = userRatings.get(i).getItem();
                ratings[i] = userRatings.get(i).getRating();
            }
            example.addLSHPoint(items, ratings);
        }

        int userIndex = 0;
        int[] top5 = example.getRecommendedItems(userIndex, 5);
        System.out.println("Recommended: " + Arrays.toString(top5));
    }
}
