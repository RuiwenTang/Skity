package com.example.skitydemo;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;



public class AuthorizationHelper {
    public static String[] PERMISSIONS_STORAGE = {
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE};

    public static boolean VerifyStoragePermissions(Activity activity,int code) {
        try {
            for (int i = 0 ; i < PERMISSIONS_STORAGE.length; ++i)
            {
                int permission = ActivityCompat.checkSelfPermission(activity,
                        PERMISSIONS_STORAGE[i]);
                if (permission != PackageManager.PERMISSION_GRANTED) {
                    ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE,code);
                    return false;
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return true;
    }
}
