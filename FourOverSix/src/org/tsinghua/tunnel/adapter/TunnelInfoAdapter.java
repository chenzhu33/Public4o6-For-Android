package org.tsinghua.tunnel.adapter;

import java.util.ArrayList;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import org.tsinghua.ti.R;

/**
 * @author carelife
 */
public class TunnelInfoAdapter extends BaseAdapter {
	private final static String TAG = "Public4o6";

	private volatile static TunnelInfoAdapter instance = null;

	private LayoutInflater layoutInflater;

	private ArrayList<String> tunnelInfoListName;

	private ArrayList<String> tunnelInfoListContent;

	public static TunnelInfoAdapter getInstance(Context ctx) {
		if (instance == null) {
			synchronized (TunnelInfoAdapter.class) {
				if (instance == null) {
					Log.d(TAG, "tunnelInfo instance is null...");
					instance = new TunnelInfoAdapter(ctx);
					instance.addTunnelInfo("Public Ipv4 Address:", "NULL");
					instance.addTunnelInfo("Public Ipv6 Address:", "NULL");
					instance.addTunnelInfo("Post set", "NULL");
					instance.addTunnelInfo("Incoming Traffic:", "0.0K");
					instance.addTunnelInfo("Outcoming Traffic:", "0.0K");
					instance.addTunnelInfo("Tunnel Status:", "CLOSE");
					instance.notifyDataSetChanged();
				}
			}
		}
		return instance;
	}

	private TunnelInfoAdapter(Context ctx, ArrayList<String> nameList,
			ArrayList<String> contentList) {
		tunnelInfoListName = nameList;
		tunnelInfoListContent = contentList;
		layoutInflater = LayoutInflater.from(ctx);
	}

	private TunnelInfoAdapter(Context ctx) {
		tunnelInfoListName = new ArrayList<String>();
		tunnelInfoListContent = new ArrayList<String>();
		layoutInflater = LayoutInflater.from(ctx);
	}

	public void addTunnelInfo(String name, String content) {
		tunnelInfoListName.add(name);
		tunnelInfoListContent.add(content);
		// this.notifyDataSetChanged();
	}

	public void updateTunnelInfo(int index, String content) {
		tunnelInfoListContent.set(index, content);
		// this.notifyDataSetChanged();
	}

	public int getCount() {
		return tunnelInfoListContent.size();
	}

	public String getItem(int position) {
		return tunnelInfoListContent.get(position);
	}

	public long getItemId(int position) {
		return position;
	}

	public View getView(int position, View convertView, ViewGroup parent) {
		ViewHolder holder = null;
		if (convertView == null) {
			convertView = layoutInflater.inflate(R.layout.tunnle_info_layout,
					null);
			holder = new ViewHolder();
			holder.headText = (TextView) convertView
					.findViewById(R.id.info_head);
			holder.contentText = (TextView) convertView
					.findViewById(R.id.info_content);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		String name = tunnelInfoListName.get(position);
		String content = tunnelInfoListContent.get(position);
		if (name != null && content != null) {
			holder.headText.setText(name);
			holder.contentText.setText(content);
		}
		return convertView;
	}

	private class ViewHolder {
		TextView headText;

		TextView contentText;
	}
}
