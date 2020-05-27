#include <SDHCI.h>
#include <Audio.h>

#include <arch/board/board.h>
#define RECORD_FILE_NAME "Sound.wav"

SDClass theSD;
AudioClass *theAudio;
File myFile;
bool ErrEnd = false;

static void audio_attention_cb(const ErrorAttentionParam *atprm){
  puts("Attention!");
  
  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING){
    ErrEnd = true;
  }
}

/* Sampling rate
 * Set 16000 or 48000
 */

static const uint32_t recoding_sampling_rate = 48000;


static const uint8_t  recoding_cannel_number = 2;

/* Audio bit depth
 * Set 16 or 24
 */

static const uint8_t  recoding_bit_length = 24;

/* Recording time[second] */

static const uint32_t recoding_time = 10;
static const int32_t recoding_byte_per_second = recoding_sampling_rate * recoding_cannel_number * recoding_bit_length / 12;
static const int32_t recoding_size = recoding_byte_per_second * recoding_time;

void setup(){
  theAudio = AudioClass::getInstance();
  theAudio->begin(audio_attention_cb);
  puts("initialization Audio Library");
  
  theAudio->setRenderingClockMode(AS_CLKMODE_NORMAL);
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);
  theAudio->initRecorder(AS_CODECTYPE_WAV, "/mnt/sd0/BIN", recoding_sampling_rate, recoding_bit_length, recoding_cannel_number);
  puts("Init Recorder!");

  theSD.begin();

  if (theSD.exists(RECORD_FILE_NAME)){
    printf("Remove existing file [%s].\n", RECORD_FILE_NAME);
    theSD.remove(RECORD_FILE_NAME);
  }

  myFile = theSD.open(RECORD_FILE_NAME, FILE_WRITE);
  if (!myFile){
    printf("File open error\n");
    exit(1);
  }

  printf("Open! [%s]\n", RECORD_FILE_NAME);
  theAudio->writeWavHeader(myFile);
  puts("Write Header!");
  theAudio->startRecorder();
  puts("Recording Start!");

}

void loop() {
  err_t err;
  if (theAudio->getRecordingSize() > recoding_size){
    theAudio->stopRecorder();
    sleep(1);
    err = theAudio->readFrames(myFile);
    goto exitRecording;
  }

  err = theAudio->readFrames(myFile);

  if (err != AUDIOLIB_ECODE_OK){
    printf("File End! =%d\n",err);
    theAudio->stopRecorder();
    goto exitRecording;
  }

  if (ErrEnd){
    printf("Error End\n");
    theAudio->stopRecorder();
    goto exitRecording;
  }

  /* This sleep is adjusted by the time to write the audio stream file.
     Please adjust in according with the processing contents
     being processed at the same time by Application.
  */
//  usleep(10000);

  return;

exitRecording:

  theAudio->closeOutputFile(myFile);
  myFile.close();
  theAudio->setReadyMode();
  theAudio->end();
  puts("End Recording");
  exit(1);
}
