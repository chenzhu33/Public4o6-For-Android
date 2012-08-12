package org.tsinghua.tunnel.adapter;

import java.util.ArrayList;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.TextView;

import org.tsinghua.ti.R;

/**
 * @author carelife
 */
public class SettingsAdapter extends BaseAdapter {
	private LayoutInflater layoutInflater;

	private Context ctx;

	private ArrayList<String> settingChooseList;

	public SettingsAdapter(Context ctx, ArrayList<String> list) {
		this.ctx = ctx;
		settingChooseList = list;
		layoutInflater = LayoutInflater.from(ctx);
	}

	public int getCount() {
		return settingChooseList.size();
	}

	public Object getItem(int position) {
		return settingChooseList.get(position);
	}

	public long getItemId(int position) {
		return position;
	}

	public View getView(int position, View convertView, ViewGroup parent) {
		ViewHolder holder = null;
		if (convertView == null) {
			convertView = layoutInflater
					.inflate(R.layout.settings_layout, null);
			holder = new ViewHolder();
			holder.headText = (TextView) convertView
					.findViewById(R.id.setting_head);
			holder.checkItem = (CheckBox) convertView
					.findViewById(R.id.setting_check);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		String info = settingChooseList.get(position);
		if (info != null) {
			holder.headText.setText(info);
		}
		return convertView;
	}

	private class ViewHolder {
		TextView headText;

		CheckBox checkItem;
	}
}
