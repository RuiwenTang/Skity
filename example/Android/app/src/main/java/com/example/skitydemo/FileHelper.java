package com.example.skitydemo;

import android.content.Context;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

public class FileHelper {

    public static boolean isSystemAssets(String fileName)
    {
        String names[] = {"device_features","huangli.idf","hybrid","images","keys","license","operators.dat","pinyinindex.idf","tel_uniqid_len8.dat","telocation.idf","webkit","xiaomi_mobile.dat"};
        for (int i = 0;i < names.length; ++i)
            if (names[i].startsWith(fileName))
                return true;
        return false;
    }

    public static void copyFilesFromAssets(Context context, String assetsPath, String savePath, boolean replaceExist){
        try {
            String fileNames[] = context.getAssets().list(assetsPath);
            if (fileNames.length > 0) {
                File file = new File(savePath);
                file.mkdirs();
                for (String fileName : fileNames) {
                    if (isSystemAssets(fileName))
                        continue;
                    String p1 = assetsPath + "/" + fileName;
                    String p2 = savePath + "/" + fileName;
                    if (assetsPath.endsWith("/"))
                        p1 = assetsPath + fileName;
                    else if (assetsPath.equals(""))
                        p1 = fileName;

                    if (savePath.endsWith("/"))
                        p2 = savePath + fileName;
                    else if (savePath.equals(""))
                        p2 = fileName;

                    copyFilesFromAssets(context, p1,p2,replaceExist);
                }
            } else {
                File savefile = new File(savePath);

                if (savefile.canWrite() && savefile.exists() && !replaceExist)
                    return;

                InputStream is = context.getAssets().open(assetsPath);
                FileOutputStream fos = new FileOutputStream(savefile);
                byte[] buffer = new byte[1024];
                int byteCount = 0;
                while ((byteCount = is.read(buffer)) != -1) {
                    // buffer
                    fos.write(buffer, 0, byteCount);
                }
                fos.flush();
                is.close();
                fos.close();
            }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

}
