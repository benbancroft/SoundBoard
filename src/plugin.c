#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <ctype.h>
#include <assert.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_definitions.h"
#include "ts3_functions.h"
#include "plugin.h"
#include "hashmap.h"

static struct TS3Functions ts3Functions;

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

#define PLUGIN_API_VERSION 20

static char* pluginID = NULL;
static hashmap *client_map;

#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result) {
	int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
	*result = (char*)malloc(outlen);
	if(WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
		*result = NULL;
		return -1;
	}
	return 0;
}
#endif

/*********************************** Required functions ************************************/
/*
 * If any of these required functions is not implemented, TS3 will refuse to load the plugin
 */

/* Unique name identifying this plugin */
const char* ts3plugin_name() {
#ifdef _WIN32
	/* TeamSpeak expects UTF-8 encoded characters. Following demonstrates a possibility how to convert UTF-16 wchar_t into UTF-8. */
	static char* result = NULL;  /* Static variable so it's allocated only once */
	if(!result) {
		const wchar_t* name = L"Test Plugin";
		if(wcharToUtf8(name, &result) == -1) {  /* Convert name into UTF-8 encoded result */
			result = "Test Plugin";  /* Conversion failed, fallback here */
		}
	}
	return result;
#else
	return "Soundboard Exporter Plugin";
#endif
}

/* Plugin version */
const char* ts3plugin_version() {
    return "1.0";
}

/* Plugin API version. Must be the same as the clients API major version, else the plugin fails to load. */
int ts3plugin_apiVersion() {
	return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "Ben Bancroft";
}

/* Plugin description */
const char* ts3plugin_description() {
	/* If you want to use wchar_t, see ts3plugin_name() on how to use */
    return "SoundBoard clip exporter plugin.";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs) {
    ts3Functions = funcs;
}

/*
 * Custom code called right after loading the plugin. Returns 0 on success, 1 on failure.
 * If the function returns 1 on failure, the plugin will be unloaded again.
 */
int ts3plugin_init() {

    /* create hashmap for audio files. */
    hashmap_init(0, &client_map);

    return 0;
}

/* Custom code called right before the plugin is unloaded */
void ts3plugin_shutdown() {
    /* Your plugin cleanup code here */
    printf("Soundboard: shutdown\n");

	/*
	 * Note:
	 * If your plugin implements a settings dialog, it must be closed and deleted here, else the
	 * TeamSpeak client will most likely crash (DLL removed but dialog from DLL code still open).
	 */

    /* destroy hashmap for audio files. */
    hashmap_iter(client_map, (hashmap_callback)remove_user, NULL);
    hashmap_destroy(client_map);

	/* Free pluginID if we registered it */
	if(pluginID) {
		free(pluginID);
		pluginID = NULL;
	}
}

/****************************** Optional functions ********************************/
/*
 * Following functions are optional, if not needed you don't need to implement them.
 */

/* Tell client if plugin offers a configuration window. If this function is not implemented, it's an assumed "does not offer" (PLUGIN_OFFERS_NO_CONFIGURE). */
int ts3plugin_offersConfigure() {
	printf("Soundboard: offersConfigure\n");
	/*
	 * Return values:
	 * PLUGIN_OFFERS_NO_CONFIGURE         - Plugin does not implement ts3plugin_configure
	 * PLUGIN_OFFERS_CONFIGURE_NEW_THREAD - Plugin does implement ts3plugin_configure and requests to run this function in an own thread
	 * PLUGIN_OFFERS_CONFIGURE_QT_THREAD  - Plugin does implement ts3plugin_configure and requests to run this function in the Qt GUI thread
	 */
	return PLUGIN_OFFERS_NO_CONFIGURE;  /* In this case ts3plugin_configure does not need to be implemented */
}

/* Plugin might offer a configuration window. If ts3plugin_offersConfigure returns 0, this function does not need to be implemented. */
void ts3plugin_configure(void* handle, void* qParentWidget) {
    printf("Soundboard: configure\n");
}

/*
 * If the plugin wants to use error return codes, plugin commands, hotkeys or menu items, it needs to register a command ID. This function will be
 * automatically called after the plugin was initialized. This function is optional. If you don't use these features, this function can be omitted.
 * Note the passed pluginID parameter is no longer valid after calling this function, so you must copy it and store it in the plugin.
 */
void ts3plugin_registerPluginID(const char* id) {
	const size_t sz = strlen(id) + 1;
	pluginID = (char*)malloc(sz * sizeof(char));
	_strcpy(pluginID, sz, id);  /* The id buffer will invalidate after exiting this function */
	printf("Soundboard: registerPluginID: %s\n", pluginID);
}

/* Plugin command keyword. Return NULL or "" if not used. */
const char* ts3plugin_commandKeyword() {
	return NULL;
}

/* Plugin processes console command. Return 0 if plugin handled the command, 1 if not handled. */
int ts3plugin_processCommand(uint64 serverConnectionHandlerID, const char* command) {
	return 1;
}

/*
 * Implement the following three functions when the plugin should display a line in the server/channel/client info.
 * If any of ts3plugin_infoTitle, ts3plugin_infoData or ts3plugin_freeMemory is missing, the info text will not be displayed.
 */

/* Static title shown in the left column in the info frame */
const char* ts3plugin_infoTitle() {
	return "Soundboard Exporter";
}

/* Required to release the memory for parameter "data" allocated in ts3plugin_infoData and ts3plugin_initMenus */
void ts3plugin_freeMemory(void* data) {
	free(data);
}

/*
 * Plugin requests to be always automatically loaded by the TeamSpeak 3 client unless
 * the user manually disabled it in the plugin dialog.
 * This function is optional. If missing, no autoload is assumed.
 */
int ts3plugin_requestAutoload() {
	return 1;  /* 1 = request autoloaded, 0 = do not request autoload */
}

/* Helper functions */

void write_little_endian(unsigned int word, int num_bytes, FILE *wav_file) {
    unsigned buf;
    while(num_bytes>0)
    {   buf = word & 0xff;
        fwrite(&buf, 1,1, wav_file);
        num_bytes--;
        word >>= 8;
    }
}

uint64_t get_time_stamp() {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}

void end_talk_state(struct UserData *data){
    if (data->wav_file != NULL){

        if (data->talk_state) {

            fseek(data->wav_file, 0, SEEK_SET);

            int bytes_per_sample = 2;
            int sample_rate = 48000;
            int byte_rate = sample_rate * data->channels * bytes_per_sample;

            // write RIFF header
            fwrite("RIFF", 1, 4, data->wav_file);
            write_little_endian(36 + bytes_per_sample * data->sample_count * data->channels, 4, data->wav_file);
            fwrite("WAVE", 1, 4, data->wav_file);

            // write fmt  subchunk
            fwrite("fmt ", 1, 4, data->wav_file);
            write_little_endian(16, 4, data->wav_file);   //SubChunk1Size is 16
            write_little_endian(1, 2, data->wav_file);    // PCM is format 1
            write_little_endian(data->channels, 2, data->wav_file);
            write_little_endian(sample_rate, 4, data->wav_file);
            write_little_endian(byte_rate, 4, data->wav_file);
            write_little_endian(data->channels * bytes_per_sample, 2, data->wav_file);  // block align
            write_little_endian(8 * bytes_per_sample, 2, data->wav_file);  // bits/sample

            fwrite("data", 1, 4, data->wav_file);
            write_little_endian(bytes_per_sample * data->sample_count * data->channels, 4, data->wav_file);

            fclose(data->wav_file);

        }
    }
    data->channels = 0;
    data->wav_file = NULL;
    data->talk_state = false;
}

int remove_user(void *data, const int key, struct UserData* value){
    end_talk_state(value);
    free(value);

    return 0;
}

int encode_base64(char *s) {

    int i = 0;

    while (s[i] != '\0'){
        switch(s[i]){
            case '+':
                s[i] = '-';
            case '/':
                s[i] = '_';
        }
        i++;
    }
    return 0;
}

/************************** TeamSpeak callbacks ***************************/

void ts3plugin_onTalkStatusChangeEvent(uint64 serverConnectionHandlerID, int status, int isReceivedWhisper, anyID clientID) {
	/* Demonstrate usage of getClientDisplayName */
	char *client_uid_base64;
    char file_name[PATH_LEN];
    struct UserData *data;
    anyID my_id;

    if (hashmap_get(client_map, clientID, (void*)&data) == -1){
        data = (struct UserData*)malloc(sizeof (struct UserData));

        data->channels = 0;
        data->wav_file = NULL;
        data->talk_state = false;
        data->sample_count = 0;

        hashmap_put(client_map, clientID, data);
    }

    if(status == STATUS_TALKING) {
        ts3Functions.getClientID(serverConnectionHandlerID, &my_id);
        if(!data->talk_state && my_id != clientID
           && ts3Functions.getClientVariableAsString(serverConnectionHandlerID, clientID, CLIENT_UNIQUE_IDENTIFIER, &client_uid_base64) == ERROR_ok
                && encode_base64(client_uid_base64) == 0) {
            snprintf(file_name, PATH_LEN, "%s/%s-%lu.wav", getenv("SOUNDBOARD"), client_uid_base64, get_time_stamp());

            if ((data->wav_file = fopen(file_name, "w")) != NULL){

                data->talk_state = true;

                //Skip 36 bytes for header later
                fseek(data->wav_file, 36, SEEK_SET);
            }
        }else{
            end_talk_state(data);
        }
    } else {
        end_talk_state(data);
    }
}

void ts3plugin_onEditPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short *samples, int sampleCount, int channels){

    struct UserData *data;
    int i;

    if (hashmap_get(client_map, clientID, (void*)&data) != -1 && data->talk_state && data->wav_file != NULL){
        /* write data */

        if (data->channels > 0 && channels != data->channels){
            printf("Soundboard: Expected No. Channels %d, actual: %d\n", data->channels, channels);
            end_talk_state(data);
            return;
        }
        data->channels = channels;

        for (i = 0; i < sampleCount; i++)
        {
            write_little_endian(samples[i], 2, data->wav_file);
        }
        data->sample_count += sampleCount;
    }
}

void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) {
    hashmap_iter(client_map, (hashmap_callback)remove_user, NULL);
    hashmap_clear(client_map);
}

