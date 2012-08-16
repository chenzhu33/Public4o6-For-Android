package org.tsinghua.tunnel.database;

import android.content.Context;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

//class help to use database
public class DBAdapter {
	public static final String KEY1_ROWID = "_id1";
	public static final String KEY1_TYPE = "type";
	public static final String KEY1_DATE = "date";
	public static final String KEY1_CONTENT = "content";
	
	public static final String KEY2_ROWID = "_id2";
	public static final String KEY2_STATUS = "status";
	public static final String KEY2_VFADDR = "vfaddr";
	public static final String KEY2_VSADDR = "vsaddr";
	public static final String KEY2_TRAFFICIN = "tafficin";
	public static final String KEY2_TRAFFICOUT = "tafficout";

	private static final String DATABASE_NAME = "fosdb";
	private static final String DATABASE_TABLE1 = "logdb";
	private static final String DATABASE_TABLE2 = "statusdb";
	private static final String TAG = "DBAdapter";
	private static final int DATABASE_VERSION = 1;
	private static final String DATABASE_CREATE1 = "create table logdb (_id1 integer primary key autoincrement, type integer not null, date text not null, content text not null);";
	private static final String DATABASE_CREATE2 = "create table statusdb (_id2 integer primary key autoincrement, status integer not null, vfaddr text not null, vsaddr text not null, tafficin text not null, tafficout text not null);";

	private final Context context;
	private DatabaseHelper DBHelper;
	private SQLiteDatabase db;

	public DBAdapter(Context ctx) {
		this.context = ctx;
		DBHelper = new DatabaseHelper(context);
	}

	private static class DatabaseHelper extends SQLiteOpenHelper {
		DatabaseHelper(Context context) {
			super(context, DATABASE_NAME, null, DATABASE_VERSION);
		}

		public void onCreate(SQLiteDatabase db) {
			Log.d("AAA","db create");
			db.execSQL(DATABASE_CREATE1);
			db.execSQL(DATABASE_CREATE2);
		}

		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
			Log.w(TAG, "Upgrading database from version" + oldVersion + "to"
					+ newVersion + ",which will destroy all old data");
			db.execSQL("DROP TABLE IF EXISTS logdb");
			db.execSQL("DROP TABLE IF EXISTS statusdb");
			onCreate(db);
		}
	}

	public DBAdapter open() throws SQLException {
		db = DBHelper.getWritableDatabase();
		return this;
	}

	public void close() {
		DBHelper.close();
	}

	// operation of LOGS TABLE
	public long insertALog(int type, String date, String content) {
		ContentValues initialValues = new ContentValues();
		initialValues.put(KEY1_TYPE, type);
		initialValues.put(KEY1_DATE, date);
		initialValues.put(KEY1_CONTENT, content);
		return db.insert(DATABASE_TABLE1, null, initialValues);
	}

	public Cursor getAllLogs() {
		return db.query(DATABASE_TABLE1, new String[] { KEY1_TYPE, KEY1_DATE,
				KEY1_CONTENT }, null, null, null, null, null);
	}

	public Cursor getLogById(long rowID) throws SQLException {
		Cursor mCursor = db.query(true, DATABASE_TABLE1, new String[] {
				KEY1_TYPE, KEY1_DATE, KEY1_CONTENT }, "_id1=?",
				new String[] { String.valueOf(rowID) }, null, null, null, null);
		if (mCursor != null) {
			mCursor.moveToFirst();
		}
		return mCursor;
	}

	// operation of STATUS TABLE
	public long saveStatus(int status, String vfaddr, String vsaddr, String in, String out) {
		ContentValues initialValues = new ContentValues();
		initialValues.put(KEY2_STATUS, status);
		initialValues.put(KEY2_VFADDR, vfaddr);
		initialValues.put(KEY2_VSADDR, vsaddr);
		initialValues.put(KEY2_TRAFFICIN, in);
		initialValues.put(KEY2_TRAFFICOUT, out);
		return db.insert(DATABASE_TABLE2, null, initialValues);
	}

	public Cursor loadStatus() {
		Cursor mCursor = db.query(DATABASE_TABLE2, new String[] { KEY2_STATUS, KEY2_VFADDR,
				KEY2_VSADDR, KEY2_TRAFFICIN, KEY2_TRAFFICOUT }, null, null, null, null, null);
		if (mCursor != null) {
			mCursor.moveToFirst();
		}
		return mCursor;
	}
}