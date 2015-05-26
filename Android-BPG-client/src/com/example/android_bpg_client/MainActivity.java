package com.example.android_bpg_client;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

import android.app.Activity;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.text.format.DateFormat;
import android.util.Log;
import android.util.TimingLogger;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

public class MainActivity extends Activity {
	
	private ImageView m_image;
	private Button m_button;
	
	// Load library
    static {
        System.loadLibrary("bpg_decoder");
    };
    
    public static byte[] toByteArray(InputStream input) throws IOException
    {
        byte[] buffer = new byte[1024];
        int bytesRead;
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        while ((bytesRead = input.read(buffer)) != -1)
        {
            output.write(buffer, 0, bytesRead);
        }
        return output.toByteArray();
    }
    
    public Bitmap getDecodedBitmap(){
    	Bitmap bm = null;
		InputStream is = getResources().openRawResource(com.example.android_bpg_client.R.raw.picture_clock);
		try{
			byte[] byteArray = toByteArray(is);
			byte[] decBuffer = null;
			int decBufferSize = 0;
			decBuffer = DecoderWrapper.decodeBuffer(byteArray, byteArray.length);
			decBufferSize = decBuffer.length;
			if(decBuffer != null){
				bm = BitmapFactory.decodeByteArray(decBuffer, 0, decBufferSize);
			}
		}
		catch(IOException ex){
			Log.i("MainActivity", "Failed to convert image to byte array");
		}
		return bm;
    }
    
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		addListenerOnButton();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
	
	public void addListenerOnButton() {
		 
		m_image = (ImageView) findViewById(R.id.imageView1);
 
		m_button = (Button) findViewById(R.id.btnChangeImage);
		m_button.setOnClickListener(new OnClickListener() {
			 
			@Override
			public void onClick(View arg0) {
				
				TextView txtView = (TextView) findViewById(R.id.text_id);
				long startTime = System.currentTimeMillis();
				Bitmap bm = getDecodedBitmap();
				if(bm != null){
					m_image.setImageBitmap(bm);
					long stopTime = System.currentTimeMillis();
					String timeString = String.valueOf(stopTime-startTime);
					txtView.setText("Image size: " + String.valueOf(bm.getHeight()) + "x" + String.valueOf(bm.getWidth()) + 
							"\nTime to load: " + timeString + "ms" );					
				}
			}
 
		});
	}
}
