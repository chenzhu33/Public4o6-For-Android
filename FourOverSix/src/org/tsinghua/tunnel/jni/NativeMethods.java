package org.tsinghua.tunnel.jni;

import org.tsinghua.tunnel.adapter.LogsAdapter;
import org.tsinghua.tunnel.adapter.TunnelInfoAdapter;

import android.content.Context;
import android.util.Log;

public class NativeMethods {
	static {
		System.loadLibrary("tunnel_jni");
	}

	private native String getTunnelInfos();

	private native String getLogs();

	private native int conTunnel(String tcAddress, String tiAddress);

	private native int disconTunnel();

	private LogsAdapter logAdapter;

	private boolean hasDisconnected = false;

	private Context context;

	public NativeMethods(Context ctx, String tcAddr, String tiAddr) {
		context = ctx;
		logAdapter = LogsAdapter.getInstance(ctx);
		Log.d("Tunnel", tiAddr);
		if (conTunnel(tcAddr, tiAddr) < 0) {
			Log.d("Tunnel", "Init Fail.Cannot connect kernel module");
			logAdapter.addLog(LogsAdapter.ERROR,
					"Cannot connect kernel module.\nNetlink Error.");
			return;
		}
		Requester requester = new Requester();
		Thread tThread = new Thread(requester);
		tThread.start();
		Log.d("Tunnel", "Init Success. Thread Start");
		logAdapter.addLog(LogsAdapter.MODULE_INFO, "Public FourOverSix Start.");
	}

	public int disconnectTunnel(boolean flag) {
		hasDisconnected = flag;
		return disconTunnel();
	}

	private class Requester implements Runnable {
		public void run() {
			while (!hasDisconnected) {
				// =========GET TUNNEL INFO==========
				String result = getTunnelInfos();
				Log.d("Tunnel", "get_tunnel_info:" + result);
				if (result.equals("-1")) {
					Log.d("Tunnel", "Connect to km for tunnel info failed");
					logAdapter.addLog(LogsAdapter.ERROR,
							"Connect to km for tunnel info failed.");
					continue;
				}
				String[] tunnelInfos = result.split("#");
				if (tunnelInfos.length != 0) {
					TunnelInfoAdapter tnAdapter = TunnelInfoAdapter
							.getInstance(context);
					int i = 0;
					for (i = 0; i < tunnelInfos.length; i++) {
						tnAdapter.updateTunnelInfo(i, tunnelInfos[i]);
					}
					tnAdapter.updateTunnelInfo(i, "OPEN");
					tnAdapter.notifyDataSetChanged();
				}

				try {
					Thread.sleep(5000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			logAdapter.addLog(LogsAdapter.MODULE_INFO,
					"Public FourOverSix Stop.");
		}
	}
}
