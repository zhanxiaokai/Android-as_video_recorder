package media.ushow.webrtcdemo;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.TextView;
import android.widget.Toast;

import com.changba.songstudio.recording.RecordingImplType;
import com.changba.songstudio.recording.camera.preview.ChangbaRecordingPreviewScheduler;
import com.changba.songstudio.recording.camera.preview.ChangbaRecordingPreviewView;
import com.changba.songstudio.recording.camera.preview.ChangbaVideoCamera;
import com.changba.songstudio.recording.exception.RecordingStudioException;
import com.changba.songstudio.recording.exception.StartRecordingException;
import com.changba.songstudio.recording.service.PlayerService;
import com.changba.songstudio.recording.video.CommonVideoRecordingStudio;
import com.changba.songstudio.recording.video.VideoRecordingStudio;
import com.changba.songstudio.recording.video.VideoRecordingStudio.RecordingStudioStateCallback;

import java.io.File;

import media.ushow.utils.FilePathUtil;

public class VideoRecorderActivity extends Activity implements OnClickListener {
    // private static final int ONE_MINUTE = 1 * 60 * 1000;
    public static final int STANDARD_SONG = 101; // 标准伴奏
    public static final int SELF_SONG = 102; // 手机里的伴奏或者第三方下载的伴奏
    public static final int CHORUS_SONG = 103; // 标准伴奏录音后的 -伴奏
    public static final int SELF_CHORUS_SONG = 104; // 手机里或第三方伴奏录制的-伴奏
    protected static final int DECODE_MP3_FAIL = 1627;
    protected static final int CANCEL_LODING_VIEW = 632;
    public static final int UPDATE_PLAY_VOICE_PROGRESS = 730;
    public static final int STOP_PLAY_VOICE = 731;
    protected static final int START_RECORD = 627;

    protected RelativeLayout connect_tip_layout;
    protected TextView load_text_tip;

    private AlwaysMarqueeTextView mSongname;
    private ImageView img_switch_camera;
    private ImageView song_selection;
    private TextView mTimeLabel;
    private ChangbaRecordingPreviewView surfaceView;
    private ImageButton mMoreBtn;
    private Button mCompleteBtn;
    private ImageButton mCloseBtn;

    private RelativeLayout mini_player_layout;
    private ImageView close_btn;
    private ImageView process_btn;
    private TextView songname_label;
    private TextView time_label;

    private SeekBar accompanyVolumSeekBar;

    private boolean iscompleted = false;
    public static long time = 3000; // 开始 的时间，不能为零，否则前面几句歌词没有显示出来
    private long lastEnterTime = 0;
    private WakeLock wakeLock = null;

    protected static final int START_RECORD_FAIL = 16271;
    private Button btn_start;

    private Song song;


    private ChangbaVideoCamera videoCamera;
    private ChangbaRecordingPreviewScheduler previewScheduler;
    private CommonVideoRecordingStudio recordingStudio;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.record_video_surfaceview_filter_activity);
        preProcess();
        initData();
        findView();
        bindListener();
        initView();
        initRecordingStudio();
    }
    
    private void initRecordingStudio() {
        videoCamera = new ChangbaVideoCamera(this);
        previewScheduler = new ChangbaRecordingPreviewScheduler(surfaceView, videoCamera) {
            public void onPermissionDismiss(final String tip) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(VideoRecorderActivity.this, tip, Toast.LENGTH_SHORT).show();
                    }
                });
            }
        };
        recordingStudio = new CommonVideoRecordingStudio(RecordingImplType.ANDROID_PLATFORM,
                mTimeHandler, onComletionListener, recordingStudioStateCallback);
    }

    PlayerService.OnCompletionListener onComletionListener = new PlayerService.OnCompletionListener() {
        @Override
        public void onCompletion() {
            iscompleted = true;
            complete_handler.post(new Runnable() {
                @Override
                public void run() {
                    Log.i("problem", "onCompletion");
                    if (song.getType() == SongSelectionActivity.BGM_TYPE) {
                        Log.i("problem", "song.getType() == SongSelectionActivity.BGM_TYPE showMiniPlayer");
                        showMiniPlayer();
                    } else {
                        Log.i("problem", "onCompletion  hideMiniPlayer");
                        hideMiniPlayer();
                    }
                }
            });
        }
    };

    private void initData() {
        song = new Song();
        song.setArtist("陈小春");
        song.setSongId(131);
        song.setName("算你狠");
    }

    private void startRecordAndPlay() {
        try {
            recordingStudio.initRecordingResource(previewScheduler);
        } catch (RecordingStudioException e) {
            recordingStudio.destroyRecordingResource();
            return;
        }
        mHandler.sendEmptyMessageDelayed(START_RECORD, 800);
    }

    public void recordStart() {
        connect_tip_layout.setVisibility(View.VISIBLE);

        int adaptiveBitrateWindowSizeInSecs = 3;
        int adaptiveBitrateEncoderReconfigInterval = 15 * 1000;
        int adaptiveBitrateWarCntThreshold = 10;

        int width = 360;
        int height = 640;
        int bitRateKbs = 900;
        int audioSampleRate = recordingStudio.getRecordSampleRate();
        recordingStudio.startVideoRecording(FilePathUtil.getVideoRecordingFilePath(),bitRateKbs, width, height,
        		audioSampleRate,0, adaptiveBitrateWindowSizeInSecs, adaptiveBitrateEncoderReconfigInterval, 
        		adaptiveBitrateWarCntThreshold,300, 800, isUseHWEncoder);
    }

    private RecordingStudioStateCallback recordingStudioStateCallback = new RecordingStudioStateCallback() {
        @Override
        public void onConnectRTMPServerFailed() {
            isPublishing = false;
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    connect_tip_layout.setVisibility(View.GONE);
                    btn_start.setText(getResources().getString(R.string.start_live));
                    btn_start.setVisibility(View.VISIBLE);
                    btn_start.setOnClickListener(VideoRecorderActivity.this);
                    countDownTimeSec = 3;
                    initRecordingStudio();
                    mCompleteBtn.setVisibility(View.GONE);
                    Toast.makeText(VideoRecorderActivity.this, "连接RTMP服务器失败", Toast.LENGTH_SHORT).show();
                }
            });
        }

        @Override
        public void onConnectRTMPServerSuccessed() {
            isPublishing = true;
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    connect_tip_layout.setVisibility(View.GONE);
                }
            });
        }

        @Override
        public void onStartRecordingException(final StartRecordingException exception) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(VideoRecorderActivity.this, exception.getMessage(), Toast.LENGTH_SHORT)
                            .show();
                }
            });
        }

        @Override
        public void onPublishTimeOut() {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    recordStop();
                    load_text_tip.setText(getResources().getString(R.string.publish_timeout_tip));
                    connect_tip_layout.setVisibility(View.VISIBLE);
                    btn_start.setVisibility(View.VISIBLE);
                    mCompleteBtn.setVisibility(View.GONE);
                    Toast.makeText(VideoRecorderActivity.this, "推流期间，由于网络原因超时", Toast.LENGTH_SHORT).show();
                }
            });
        }

        @Override
        public void statisticsCallback(long startTimeMills, int connectTimeMills, int publishDurationInSec,
                                       float discardFrameRatio, float publishAVGBitRate, float expectedBitRate, String adaptiveBitrateChart) {
            StringBuilder statisticsBuilder = new StringBuilder("statisticsCallback : ");
            statisticsBuilder.append("startTimeMills【").append(startTimeMills).append("】");
            statisticsBuilder.append("connectTimeMills【").append(connectTimeMills).append("】");
            statisticsBuilder.append("publishDurationInSec【").append(publishDurationInSec).append("】");
            statisticsBuilder.append("discardFrameRatio【").append(discardFrameRatio).append("】");
            statisticsBuilder.append("publishAVGBitRate【").append(publishAVGBitRate).append("】");
            statisticsBuilder.append("expectedBitRate【").append(expectedBitRate).append("】");
            statisticsBuilder.append("adaptiveBitrateChart【").append(adaptiveBitrateChart).append("】");
            Log.i("problem", statisticsBuilder.toString());
        }

        @Override
        public void adaptiveVideoQuality(int videoQuality) {
            boolean invalidFlag = false;
            boolean showUserTip = false;
            int bitrate = VideoRecordingStudio.COMMON_VIDEO_BIT_RATE;
            int bitrateLimits = VideoRecordingStudio.COMMON_VIDEO_BIT_RATE;
            int fps = VideoRecordingStudio.VIDEO_FRAME_RATE;
            switch (videoQuality) {
                case VideoRecordingStudio.HIGHT_VIDEO_QUALITY:
                    bitrate = VideoRecordingStudio.COMMON_VIDEO_BIT_RATE;
                    ;
                    bitrateLimits = VideoRecordingStudio.COMMON_VIDEO_BIT_RATE;
                    ;
                    fps = VideoRecordingStudio.VIDEO_FRAME_RATE;
                    ;
                    break;
                case VideoRecordingStudio.MIDDLE_VIDEO_QUALITY:
                    bitrate = VideoRecordingStudio.MIDDLE_VIDEO_BIT_RATE;
                    bitrateLimits = VideoRecordingStudio.MIDDLE_VIDEO_BIT_RATE;
                    fps = VideoRecordingStudio.MIDDLE_VIDEO_FRAME_RATE;
                    break;
                case VideoRecordingStudio.LOW_VIDEO_QUALITY:
                    showUserTip = true;
                    bitrate = VideoRecordingStudio.LOW_VIDEO_BIT_RATE;
                    bitrateLimits = VideoRecordingStudio.LOW_VIDEO_BIT_RATE;
                    fps = VideoRecordingStudio.LOW_VIDEO_FRAME_RATE;
                    break;
                case VideoRecordingStudio.INVLAID_QUALITY:
                    invalidFlag = true;
                    break;
                default:
                    break;
            }
            if (invalidFlag) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        recordStop();
                        load_text_tip.setText(getResources().getString(R.string.publish_timeout_tip));
                        connect_tip_layout.setVisibility(View.VISIBLE);
                        btn_start.setVisibility(View.VISIBLE);
                        mCompleteBtn.setVisibility(View.GONE);
                        Toast.makeText(VideoRecorderActivity.this, "由于当前网络环境过差，无法支持视频直播。请切换至其他网络或改善所处网络环境后重新开播！", Toast.LENGTH_SHORT).show();
                    }
                });
            } else {
                Log.i("problem", "由于当前网络环境较差，已切换至流畅模式。如需使用高清模式，请改善所处网络环境后重新开播！[" + (int) (bitrate / 1024) + "Kbps, " + fps + "]");
                if (showUserTip) {
                    mHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            String text = "由于当前网络环境较差，已切换至流畅模式。如需使用高清模式，请改善所处网络环境后重新开播！";
                            new AlertDialog.Builder(VideoRecorderActivity.this).setMessage(text)
                                    .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                                        @Override
                                        public void onClick(DialogInterface dialog, int arg1) {
                                            if (null != dialog) {
                                                dialog.dismiss();
                                                dialog = null;
                                            }
                                        }
                                    }).setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                                public void onClick(DialogInterface dialog, int id) {
                                    if (null != dialog) {
                                        dialog.dismiss();
                                        dialog = null;
                                    }
                                }
                            }).show();
                        }
                    });
                }
                previewScheduler.adaptiveVideoQuality(bitrateLimits, bitrate, fps);
            }
        }

        @Override
        public void hotAdaptiveVideoQuality(int maxBitrate, int avgBitrate, int fps) {
            previewScheduler.hotConfigQuality(maxBitrate * 1000, avgBitrate * 1000, fps);

            Log.d("adaptiveVideoQuality", " maxBitrate " + maxBitrate + " fps " + fps);
        }

		@Override
		public void statisticsBitrateCallback(int sendBitrate, int compressedBitrate) {
		}
    };

    public void recordStop() {
        recordingStudio.stopRecording();
    }

    private void completeRecord() {
        recordStop();
//		Intent intent = new Intent(VideoRecorderActivity.this, ChangbaPlayerActivity.class);
//		startActivity(intent);
        finish();
    }

    private void preProcess() {
        // 保持屏幕长亮
        wakeLock = ((PowerManager) getSystemService(Context.POWER_SERVICE)).newWakeLock(PowerManager.FULL_WAKE_LOCK,
                "RecordActivity");
        wakeLock.acquire();
    }

    private RelativeLayout recordScreen;

    private void findView() {
        recordScreen = (RelativeLayout) findViewById(R.id.recordscreen);
        connect_tip_layout = (RelativeLayout) findViewById(R.id.connect_tip_layout);
        load_text_tip = (TextView) findViewById(R.id.load_text_tip);
        load_text_tip.setText(getResources().getString(R.string.connect_rtmp_server));

        mTimeLabel = (TextView) findViewById(R.id.timelabel);
        mSongname = (AlwaysMarqueeTextView) findViewById(R.id.songname);
        mCloseBtn = (ImageButton) findViewById(R.id.imagebutton_goback);
        btn_start = (Button) this.findViewById(R.id.btn_start);
        mCompleteBtn = (Button) this.findViewById(R.id.btn_complete);
        mMoreBtn = (ImageButton) this.findViewById(R.id.btn_more);
        img_switch_camera = (ImageView) this.findViewById(R.id.img_switch_camera);
        song_selection = (ImageView) this.findViewById(R.id.song_selection);
        surfaceView = new ChangbaRecordingPreviewView(this);
        recordScreen.addView(surfaceView, 0);
        surfaceView.getLayoutParams().width = getWindowManager().getDefaultDisplay().getWidth();
        surfaceView.getLayoutParams().height = getWindowManager().getDefaultDisplay().getWidth();


        // Mini Player
        mini_player_layout = (RelativeLayout) findViewById(R.id.mini_player_layout);
        close_btn = (ImageView) mini_player_layout.findViewById(R.id.close_btn);
        process_btn = (ImageView) mini_player_layout.findViewById(R.id.process_btn);
        songname_label = (TextView) mini_player_layout.findViewById(R.id.songname_label);
        time_label = (TextView) mini_player_layout.findViewById(R.id.time_label);
        mini_player_layout.setVisibility(View.GONE);

        //Volume And PitchShift
        accompanyVolumSeekBar = (SeekBar) findViewById(R.id.accompany_seek_bar);
    }

    private void initView() {
        ((RelativeLayout) findViewById(R.id.album_box_front)).getLayoutParams().height = getWindowManager()
                .getDefaultDisplay().getWidth();
        mSongname.setText("推流");
    }

    private void bindListener() {
        mCloseBtn.setOnClickListener(this);
        mCompleteBtn.setOnClickListener(this);
        btn_start.setOnClickListener(this);
        mMoreBtn.setOnClickListener(this);
        img_switch_camera.setOnClickListener(this);
        song_selection.setOnClickListener(this);
        process_btn.setOnClickListener(this);
        close_btn.setOnClickListener(this);

        accompanyVolumSeekBar.setOnSeekBarChangeListener(accompanyVolumeSeekbarChangeListener);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (wakeLock != null)
            wakeLock.release();
    }

    @Override
    protected void onResume() {
        super.onResume();
        // 监测耳机的拔出或者插入操作
        if (wakeLock != null)
            wakeLock.acquire();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            return closeWindow();
        }
        return super.onKeyDown(keyCode, event);
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case START_RECORD:
                    mMoreBtn.setEnabled(false);
                    recordStart();
                    break;
            }
        }
    };

    private boolean isPlaying;

    private static final int ONE_HOUR_TIME_MILLS = 1 * 60 * 60 * 1000;
    int totalDuration = 0;
    int currentTimeMs = 0;
    private Handler mTimeHandler = new Handler() {
        public void handleMessage(Message msg) {
            try {
                // 计算当前时间
                currentTimeMs = Math.max(msg.arg1, 0);
                int publishTimeInSecs = Math.max(msg.arg1, 0) / 1000;
                int accompanyTimeInSecs = Math.max(msg.arg2, 0) / 1000;
                renderPublishTimeLabel(publishTimeInSecs, ONE_HOUR_TIME_MILLS);
                if (isPlaying) {
                    renderMiniPlayerTimeLabel(accompanyTimeInSecs, totalDuration);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    };

    private void renderMiniPlayerTimeLabel(int timeMills, int totalDuration) {
        String curtime = String.format("%02d:%02d", timeMills / 60, timeMills % 60);
        int _totalTime = totalDuration / 1000;
        String totalTime = String.format("%02d:%02d", _totalTime / 60, _totalTime % 60);
        String timeLabelText = curtime + "/" + totalTime;
        time_label.setText(timeLabelText);
    }

    private void renderPublishTimeLabel(int timeMills, int totalDuration) {
        String curtime = String.format("%02d:%02d", timeMills / 60, timeMills % 60);
        int _totalTime = totalDuration / 1000;
        String totalTime = String.format("%02d:%02d", _totalTime / 60, _totalTime % 60);
        String timeLabelText = curtime + "/" + totalTime;
        timeLabelText = "正在发布 " + timeLabelText;
        mTimeLabel.setText(timeLabelText);
    }

    Handler complete_handler = new Handler() {
        public void handleMessage(Message msg) {
            if (iscompleted) {
                completeRecord();
            } else {
                String text = "当前歌曲还没有结束，确认要完成录制吗？";
                new AlertDialog.Builder(VideoRecorderActivity.this).setMessage(text)
                        .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int arg1) {
                                if (null != dialog) {
                                    dialog.dismiss();
                                    dialog = null;
                                }
                                Toast.makeText(VideoRecorderActivity.this, "处理中", Toast.LENGTH_SHORT).show();
                                completeRecord();
                            }
                        }).setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        if (null != dialog) {
                            dialog.dismiss();
                            dialog = null;
                        }
                    }
                }).show();
            }
        }
    };

    private int countDownTimeSec = 3;
    private boolean isUseHWEncoder = true;

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (SONG_SELECTION_REQUEST_CODE == requestCode) {
            switch (resultCode) { // resultCode为回传的标记，我在B中回传的是RESULT_OK
                case RESULT_OK:
                    Bundle b = data.getExtras(); // data为B中回传的Intent
                    song = (Song) b.getSerializable(SongSelectionActivity.SONG_ID);
                    if (null != song) {
                        showMiniPlayer();
                    }
                    break;
                default:
                    break;
            }
        }
    }

    private boolean isPublishing = false;

    private void showMiniPlayer() {
        if (!isPublishing) {
            return;
        }
        mini_player_layout.setVisibility(View.VISIBLE);
        process_btn.setImageResource(R.drawable.pause_button);
        songname_label.setText(song.getName() + "-" + song.getArtist());
        String musicPath = song.getSongFilePath("music");
        MediaPlayer mediaPlayer;
        try {
            mediaPlayer = MediaPlayer.create(this, Uri.fromFile(new File(musicPath)));
            totalDuration = Math.max(mediaPlayer.getDuration(), 0);
            mediaPlayer.release();
        } catch (Exception e) {
            e.printStackTrace();
        }
        recordingStudio.startAccompany(musicPath);
        isPlaying = true;
    }

    private void pauseMiniPlayer() {
        recordingStudio.pauseAccompany();
    }

    private void resumeMiniPlayer() {
        recordingStudio.resumeAccompany();
    }

    private void hideMiniPlayer() {
        mini_player_layout.setVisibility(View.GONE);
        process_btn.setImageResource(R.drawable.play_button);
        isPlaying = false;
    }

    private void closeMiniPlayer() {
        recordingStudio.stopAccompany();
        this.hideMiniPlayer();
    }

    private static final int SONG_SELECTION_REQUEST_CODE = 10001;

    @Override
    public void onClick(View v) {
        int btnId = v.getId();
        switch (btnId) {
            case R.id.close_btn:
                closeMiniPlayer();
                break;
            case R.id.process_btn:
                if (isPlaying) {
                    isPlaying = false;
                    pauseMiniPlayer();
                    process_btn.setImageResource(R.drawable.play_button);
                } else {
                    isPlaying = true;
                    resumeMiniPlayer();
                    process_btn.setImageResource(R.drawable.pause_button);
                }
                break;
            case R.id.song_selection:
                Intent intent = new Intent();
                intent.setClass(VideoRecorderActivity.this, SongSelectionActivity.class);
                startActivityForResult(intent, SONG_SELECTION_REQUEST_CODE);
                break;
            case R.id.img_switch_camera:
                previewScheduler.switchCameraFacing();
                break;
            case R.id.btn_more:
                if (!isUseHWEncoder) {
                    isUseHWEncoder = true;
                    Toast.makeText(VideoRecorderActivity.this, "已经切换为硬件编码", Toast.LENGTH_LONG).show();
                } else {
                    isUseHWEncoder = false;
                    Toast.makeText(VideoRecorderActivity.this, "已经切换为软件编码", Toast.LENGTH_LONG).show();
                }

                break;
            case R.id.imagebutton_goback:
                closeWindow();
                break;
            case R.id.btn_start:
                btn_start.setOnClickListener(null);
                // 倒计时5秒开始录制
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (isFinishing()) {
                            return;
                        }
                        if (countDownTimeSec >= 1) {
                            btn_start.setText(countDownTimeSec + "");
                            mHandler.postDelayed(this, 1000);
                        }
                        if (countDownTimeSec == 0) {
                            btn_start.setVisibility(View.GONE);
                            mCompleteBtn.setVisibility(View.VISIBLE);
                        }
                        if (countDownTimeSec == 1) {
                            // 开始放伴奏加录音加录制视频
                            startRecordAndPlay();
                        }
                        countDownTimeSec--;
                    }
                });
                break;
            case R.id.btn_complete:
                complete_handler.sendEmptyMessage(0);
                break;
        }
    }

    private boolean closeWindow() {
        if ((System.currentTimeMillis() - lastEnterTime) < 1800) {
            Toast.makeText(VideoRecorderActivity.this, "操作过于频繁,请稍候再试", Toast.LENGTH_SHORT).show();
            return true;
        }
        lastEnterTime = System.currentTimeMillis();
        String text = "当前歌曲还没有结束，确认要取消录制吗？";
        new AlertDialog.Builder(VideoRecorderActivity.this).setMessage(text)
                .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int arg1) {
                        if (null != dialog) {
                            dialog.dismiss();
                            dialog = null;
                        }
                        Toast.makeText(VideoRecorderActivity.this, "结束中", Toast.LENGTH_SHORT).show();
                        recordStop();
                        finish();
                    }
                }).setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                if (null != dialog) {
                    dialog.dismiss();
                    dialog = null;
                }
            }
        }).show();
        return true;
    }


    private float accompanyVolume = 1.0f;
    private static final int ACCOMPANY_VOLUME_CHANGED = 1098703;
    private static final int AUDIO_VOLUME_CHANGED = 1098704;
    private static final int PITCH_LEVEL_CHANGED = 1098705;

    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case AUDIO_VOLUME_CHANGED:
                    break;
                case ACCOMPANY_VOLUME_CHANGED:
                    if (null != recordingStudio) {
                        recordingStudio.setAccompanyVolume(accompanyVolume, 1.0f);
                    }
                    break;
                case PITCH_LEVEL_CHANGED:
                    break;
            }
        }
    };

    private OnSeekBarChangeListener accompanyVolumeSeekbarChangeListener = new OnSeekBarChangeListener() {
        @Override
        public void onStopTrackingTouch(SeekBar seekBar) {

        }

        @Override
        public void onStartTrackingTouch(SeekBar seekBar) {
        }

        @Override
        public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            float pro = seekBar.getProgress();
            float num = seekBar.getMax();
            float result = ((pro - 50) / num) * 2 + 1.0f;
            if (result < 0.0f) {
                result = 0.0f;
            }
            accompanyVolume = result;
            handler.sendEmptyMessage(ACCOMPANY_VOLUME_CHANGED);
        }
    };
}