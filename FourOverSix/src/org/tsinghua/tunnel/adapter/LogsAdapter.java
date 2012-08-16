package org.tsinghua.tunnel.adapter;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import android.content.Context;
import android.database.Cursor;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import org.tsinghua.ti.R;
import org.tsinghua.tunnel.database.DBAdapter;

/**
 * @author carelife
 */
public class LogsAdapter extends BaseAdapter {
	public static int ERROR = 0;
	public static int MODULE_INFO = 1;
	public static int DHCP_INFO = 2;
	public static int OTHER = 3;

	private volatile static LogsAdapter instance = null;

	private LayoutInflater layoutInflater;

	private Context context;

	private ArrayList<Log> logsList;

	private DBAdapter db;

	private SparseArray<String> logTypeHash = new SparseArray<String>();

	public static LogsAdapter getInstance(Context ctx) {
		if (instance == null) {
			synchronized (LogsAdapter.class) {
				if (instance == null) {
					instance = new LogsAdapter(ctx);
				}
			}
		}
		return instance;
	}

	public LogsAdapter(Context ctx) {
		this.context = ctx;
		logTypeHash.append(ERROR, "Error");
		logTypeHash.append(MODULE_INFO, "Module_Info");
		logTypeHash.append(DHCP_INFO, "DHCP_Info");
		logTypeHash.append(OTHER, "Other");

		if (logsList == null) {
			logsList = new ArrayList<Log>();
			db = new DBAdapter(ctx);
			db.open();
			Cursor cur = db.getAllLogs();
			if (cur.getCount() != 0) {
				cur.moveToFirst();
				do {
					android.util.Log.d("AAA", cur.getInt(0) + cur.getString(1)
							+ cur.getString(2));
					Log log = new Log(cur.getInt(0), cur.getString(1),
							cur.getString(2));
					logsList.add(log);
				} while (cur.moveToNext());
			}
			cur.close();
			db.close();
		}
		layoutInflater = LayoutInflater.from(ctx);
	}

	public void addLog(int type, String content) {
		SimpleDateFormat sDateFormat = new SimpleDateFormat(
				"yyyy-MM-dd hh:mm:ss");
		String date = sDateFormat.format(new java.util.Date());
		Log log = new Log(type, date, content);
		logsList.add(log);
		this.notifyDataSetChanged();
		db.open();
		db.insertALog(type, date, content);
		db.close();
	}

	public int getCount() {
		return logsList.size();
	}

	public Object getItem(int position) {
		return logsList.get(position);
	}

	public long getItemId(int position) {
		return position;
	}

	public View getView(int position, View convertView, ViewGroup parent) {
		ViewHolder holder = null;
		if (convertView == null) {
			convertView = layoutInflater.inflate(R.layout.logs_layout, null);
			holder = new ViewHolder();
			holder.dateText = (TextView) convertView
					.findViewById(R.id.log_date);
			holder.typeText = (TextView) convertView
					.findViewById(R.id.log_type);
			holder.contentText = (TextView) convertView
					.findViewById(R.id.log_content);
			convertView.setTag(holder);
		} else {
			holder = (ViewHolder) convertView.getTag();
		}
		Log log = logsList.get(position);
		if (log != null) {
			holder.dateText.setText(log.date);
			holder.typeText.setText(logTypeHash.get(log.type));
			holder.contentText.setText(log.content);
		}
		return convertView;
	}

	private class Log {
		public int type;
		public String date;
		public String content;

		public Log(int type, String date, String content) {
			this.type = type;
			this.date = date;
			this.content = content;
		}
	}

	private class ViewHolder {
		TextView dateText;
		TextView typeText;
		TextView contentText;
	}
}
