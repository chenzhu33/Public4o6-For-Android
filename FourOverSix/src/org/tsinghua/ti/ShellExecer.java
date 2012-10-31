package org.tsinghua.ti;

import java.io.*;
import java.util.ArrayList;

import android.content.Context;
import android.util.Log;

public class ShellExecer {

	private static Context context;

	public static String kmFileName = "public4over6.ko";

	public static String kmName = "public4over6";

	public static String insmodShellName = "insmod";

	public static String rmmodShellName = "rmmod";

	private static String FILEPATH = "/data/data/org.tsinghua.ti/module/";

	private static ArrayList<String> loadCmd = new ArrayList<String>();
	
	private static void copyFile(String filename) throws IOException {
		String fileNames = FILEPATH + filename;
		File dir = new File(FILEPATH);
		if (!dir.exists())
			dir.mkdir();
		FileOutputStream os = null;
		try {
			os = new FileOutputStream(fileNames);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		InputStream is;
		if (filename.equals(kmFileName)) {
			is = context.getResources().openRawResource(R.raw.public4over6);
		} else {
			is = null;
		}
		byte[] buffer = new byte[8192];
		int count = 0;
		try {
			while ((count = is.read(buffer)) > 0) {
				os.write(buffer, 0, count);
				os.flush();
			}
		} catch (IOException e) {

		}
		try {
			is.close();
			os.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public static void init(Context mcontext) {
		context = mcontext;
		try {
			copyFile(kmFileName);
		} catch (IOException e) {
			e.printStackTrace();
		}
		loadCmd.add("ifconfig public4over6 up");
		loadCmd.add("ip addr add 58.205.200.18/24 broadcast 58.205.200.255 dev public4over6");
		loadCmd.add("ip route del default dev wlan0");
		loadCmd.add("ip route add default dev public4over6 metric 10");
		loadCmd.add("ip route add default dev wlan0 metric 100");

	}

	public static void execCommand(int command) throws IOException {
		String load0;
		switch (command) {
		case 0:// insmod
			load0 = insmodShellName + " " + FILEPATH + kmFileName;
			runRootCommand(load0);
			for(String cmd : loadCmd) {
				runRootCommand(cmd);
			}
			break;
		case 1:// rmmod
			load0 = rmmodShellName + " " + kmName;
			runRootCommand(load0);
			break;
		default:
			load0 = "";
			break;
		}
	}

	private static boolean runRootCommand(String command) {
		Process process = null;
		DataOutputStream os = null;
		try {
			process = Runtime.getRuntime().exec("su");
			os = new DataOutputStream(process.getOutputStream());
			os.writeBytes(command + "\n");
			os.writeBytes("exit\n");
			os.flush();
			process.waitFor();
		} catch (Exception e) {
			Log.d("Tunnel",
					"the  device is not rooted, error message: "
							+ e.getMessage());
			return false;
		} finally {
			try {
				if (os != null) {
					os.close();
				}
				if (process != null) {
					process.destroy();
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
		return true;
	}
}
