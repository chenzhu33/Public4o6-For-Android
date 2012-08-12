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
	public static final String KEY_ROWID = "_id1";
	public static final String KEY_TYPE = "type";
	public static final String KEY_DATE = "date";
	public static final String KEY_CONTENT = "content";

	private static final String DATABASE_NAME = "fosdb";
	private static final String DATABASE_TABLE1 = "logdb";
	private static final String TAG = "DBAdapter";
	private static final int DATABASE_VERSION = 1;
	private static final String DATABASE_CREATE1 = "create table logdb (_id1 integer primary key autoincrement, type integer not null, date text not null, content text not null);";

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
			db.execSQL(DATABASE_CREATE1);
		}

		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
			Log.w(TAG, "Upgrading database from version" + oldVersion + "to"
					+ newVersion + ",which will destroy all old data");
			db.execSQL("DROP TABLE IF EXISTS logdb");
			// db.execSQL("DROP TABLE IF EXISTS aidtocon");
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
		initialValues.put(KEY_TYPE, type);
		initialValues.put(KEY_DATE, date);
		initialValues.put(KEY_CONTENT, content);
		return db.insert(DATABASE_TABLE1, null, initialValues);
	}

	public Cursor getAllLogs() {
		return db.query(DATABASE_TABLE1, new String[] { KEY_TYPE, KEY_DATE,
				KEY_CONTENT }, null, null, null, null, null);
	}

	public Cursor getLogById(long rowID) throws SQLException {
		Cursor mCursor = db.query(true, DATABASE_TABLE1, new String[] {
				KEY_TYPE, KEY_DATE, KEY_CONTENT }, "_id1=?",
				new String[] { String.valueOf(rowID) }, null, null, null, null);
		if (mCursor != null) {
			mCursor.moveToFirst();
		}
		return mCursor;
	}

}