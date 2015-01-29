package iu.slam.utils;

import com.google.gson.Gson;

/**
 *
 * @author hanaldo
 */
public class MyGson {

    private static Gson GSON;

    public static Gson getGSON() {
        if (GSON == null) {
            GSON = new Gson();
        }
        return GSON;
    }

    public static void clean() {
        GSON = null;
    }

    private MyGson() {
    }
}
