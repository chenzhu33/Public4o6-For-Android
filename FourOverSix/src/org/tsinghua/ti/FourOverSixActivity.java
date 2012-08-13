package org.tsinghua.ti;

import java.io.IOException;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;

import org.tsinghua.ti.R;

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.Toast;

import org.tsinghua.tunnel.adapter.LogsAdapter;
import org.tsinghua.tunnel.adapter.SettingsAdapter;
import org.tsinghua.tunnel.adapter.TunnelInfoAdapter;
import org.tsinghua.tunnel.component.MyScrollLayout;
import org.tsinghua.tunnel.component.OnViewChangeListener;
import org.tsinghua.tunnel.jni.*;

public class FourOverSixActivity extends Activity implements
		OnViewChangeListener, OnClickListener {

	private final static String TAG = "Public4o6";

	private SettingsAdapter stAdapter;

	private TunnelInfoAdapter tnAdapter;

	private LogsAdapter logAdapter;

	private ListView tnList;

	private ListView stList;

	private ListView logList;

	private Button conButton;

	private Button disconButton;

	private EditText tcV6Address;

	private NativeMethods nativeMethods;

	private Spinner dhcpV4AddressChooser;

	private MyScrollLayout mScrollLayout;
	private ImageView[] mImageViews;
	private int mViewCount;
	private int mCurSel;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		Log.d(TAG,"balabalbalabal");
		init();
		initFile();
		initGui();
	}

	private void init() {
		mScrollLayout = (MyScrollLayout) findViewById(R.id.ScrollLayout);
		LinearLayout linearLayout = (LinearLayout) findViewById(R.id.llayout);
		mViewCount = mScrollLayout.getChildCount();
		mImageViews = new ImageView[mViewCount];
		for (int i = 0; i < mViewCount; i++) {
			mImageViews[i] = (ImageView) linearLayout.getChildAt(i);
			mImageViews[i].setEnabled(true);
			mImageViews[i].setOnClickListener(this);
			mImageViews[i].setTag(i);
		}
		mCurSel = 0;
		mImageViews[mCurSel].setEnabled(false);
		mScrollLayout.SetOnViewChangeListener(this);
	}

	private void initFile() {
		ShellExecer.init(FourOverSixActivity.this);
	}

	private void initGui() {
		ArrayList<String> initSettingInfo = new ArrayList<String>();
		ArrayList<String> dhcpV4AddressList = new ArrayList<String>();

		initSettingInfo.add("Show tunnel status");
		initSettingInfo.add("Auto startUp");
		initSettingInfo.add("Record logs");

		dhcpV4AddressList.add("58.205.200.10");
		dhcpV4AddressList.add("58.205.200.18");
		dhcpV4AddressList.add("58.205.200.32");

		stAdapter = new SettingsAdapter(this, initSettingInfo);

		tnAdapter = TunnelInfoAdapter.getInstance(this);

		logAdapter = LogsAdapter.getInstance(this);

		stList = (ListView) findViewById(R.id.settings);
		stList.setAdapter(stAdapter);
		tnList = (ListView) findViewById(R.id.tunnel_info);
		tnList.setAdapter(tnAdapter);
		logList = (ListView) findViewById(R.id.logs_info);
		logList.setAdapter(logAdapter);

		tcV6Address = (EditText) findViewById(R.id.tc_ipv6_addr);
		tcV6Address.setText("2001:0da8:020d:0027:7a2b:cbff:fe1b:6ce0");
		dhcpV4AddressChooser = (Spinner) findViewById(R.id.dhcp_ipv4_addr);
		ArrayAdapter<String> dhcpAdapter = new ArrayAdapter<String>(
				FourOverSixActivity.this, android.R.layout.simple_spinner_item,
				dhcpV4AddressList);
		dhcpV4AddressChooser.setAdapter(dhcpAdapter);

		conButton = (Button) findViewById(R.id.connect);
		disconButton = (Button) findViewById(R.id.disconnect);
		conButton.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				try {
					LogsAdapter.getInstance(FourOverSixActivity.this).addLog(
							LogsAdapter.MODULE_INFO, "Start insmod module...");
					ShellExecer.execCommand(0);// TODO insmod shell location
					Log.d("Tunnel", "insmod success");
				} catch (IOException e) {
					e.printStackTrace();
				}
				nativeMethods = new NativeMethods(FourOverSixActivity.this,
						tcV6Address.getText().toString(), getLocalIP());

				Toast.makeText(FourOverSixActivity.this, "Connection Success",
						Toast.LENGTH_SHORT).show();
			}
		});
		disconButton.setOnClickListener(new OnClickListener() {
			public void onClick(View v) {
				try {
					ShellExecer.execCommand(1);// TODO rmmod shell location
					Log.d("Tunnel", "rmmod success");
				} catch (IOException e) {
					e.printStackTrace();
				}
				if (-1 == nativeMethods.disconnectTunnel(true)) {
					Toast.makeText(FourOverSixActivity.this,
							"DisConnection Failed", Toast.LENGTH_SHORT).show();
				} else {
					Toast.makeText(FourOverSixActivity.this,
							"DisConnection Success", Toast.LENGTH_SHORT).show();
				}
			}
		});
	}

	private void showNotification() {
		NotificationManager notificationManager = (NotificationManager) this
				.getSystemService(android.content.Context.NOTIFICATION_SERVICE);

		Notification notification = new Notification(R.drawable.ic_launcher,
				TAG, System.currentTimeMillis());
		notification.flags |= Notification.FLAG_ONGOING_EVENT;
		notification.flags |= Notification.FLAG_NO_CLEAR;
		notification.flags |= Notification.FLAG_SHOW_LIGHTS;
		notification.defaults = Notification.DEFAULT_LIGHTS;
		notification.ledARGB = Color.BLUE;
		notification.ledOnMS = 5000;

		CharSequence contentTitle = "Public4o6 is running...";
		CharSequence contentText = "Traffic In: 0.0KBytes\nTraffic Out: 0.0KBytes";
		
		Intent notificationIntent = new Intent(this, FourOverSixActivity.class);
		PendingIntent contentItent = PendingIntent.getActivity(this, 0,
				notificationIntent, 0);
		notification.setLatestEventInfo(this, contentTitle, contentText,
				contentItent);

		notificationManager.notify(0, notification);
	}

	@Override
	public void onStop() {
		Log.d(TAG,"OnStop...");
		showNotification();
		super.onStop();
	}

//	@Override
//	public void onResume() {
//		Log.d(TAG,"OnResume...");
//		super.onResume();
//	}
//	
//	@Override
//	public void onDestroy() {
//		Log.d(TAG,"OnDestroy...");
//		super.onDestroy();
//	}
//	
//	@Override
//	public void onStart() {
//		Log.d(TAG,"OnStart...");
//		super.onStart();
//	}
//	
//	@Override
//	public void onRestart() {
//		Log.d(TAG,"OnRestart...");
//		super.onRestart();
//	}

	private void doExit() {
		NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
		nm.cancel(R.string.app_name);
	}

	@SuppressWarnings("rawtypes")
	private String getLocalIP() {
		String ipv6Address = null;
		try {
			Enumeration e1 = (Enumeration) NetworkInterface
					.getNetworkInterfaces();
			while (e1.hasMoreElements()) {
				NetworkInterface ni = (NetworkInterface) e1.nextElement();
				if (ni.getName().equals("wlan0")) {
					Enumeration e2 = ni.getInetAddresses();
					while (e2.hasMoreElements()) {
						InetAddress ia = (InetAddress) e2.nextElement();
						if (ia instanceof Inet4Address)
							continue; // omit IPv4 address
						if (!ia.getHostAddress().startsWith("fe80")) {
							ipv6Address = ia.getHostAddress();
							break;
						}
					}
				}
			}
		} catch (SocketException e) {
			e.printStackTrace();
		}
		if (ipv6Address != null)
			return ipv6Address.substring(0, ipv6Address.indexOf("%"));
		return null;
	}

	private void setCurPoint(int index) {
		if (index < 0 || index > mViewCount - 1 || mCurSel == index) {
			return;
		}
		mImageViews[mCurSel].setEnabled(true);
		mImageViews[index].setEnabled(false);
		mCurSel = index;
	}

	public void OnViewChange(int view) {
		setCurPoint(view);
	}

	public void onClick(View v) {
		int pos = (Integer) (v.getTag());
		setCurPoint(pos);
		mScrollLayout.snapToScreen(pos);
	}

}
