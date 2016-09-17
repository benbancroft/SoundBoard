#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef WIN32
#define PLUGINS_EXPORTDLL __declspec(dllexport)
#else
#define PLUGINS_EXPORTDLL __attribute__ ((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct UserData {
    FILE* wav_file;
    bool talk_state;
    int sample_count;
    int channels;
};

#define USER_ID 512
#define PATH_LEN 1024

PLUGINS_EXPORTDLL const char* ts3plugin_name();
PLUGINS_EXPORTDLL const char* ts3plugin_version();
PLUGINS_EXPORTDLL int ts3plugin_apiVersion();
PLUGINS_EXPORTDLL const char* ts3plugin_author();
PLUGINS_EXPORTDLL const char* ts3plugin_description();
PLUGINS_EXPORTDLL void ts3plugin_setFunctionPointers(const struct TS3Functions funcs);
PLUGINS_EXPORTDLL int ts3plugin_init();
PLUGINS_EXPORTDLL void ts3plugin_shutdown();
PLUGINS_EXPORTDLL int ts3plugin_offersConfigure();
PLUGINS_EXPORTDLL void ts3plugin_configure(void* handle, void* qParentWidget);
PLUGINS_EXPORTDLL void ts3plugin_registerPluginID(const char* id);
PLUGINS_EXPORTDLL const char* ts3plugin_commandKeyword();
PLUGINS_EXPORTDLL int ts3plugin_processCommand(uint64 serverConnectionHandlerID, const char* command);
PLUGINS_EXPORTDLL const char* ts3plugin_infoTitle();
PLUGINS_EXPORTDLL void ts3plugin_freeMemory(void* data);
PLUGINS_EXPORTDLL int ts3plugin_requestAutoload();

/* Helper functions */
void write_little_endian(unsigned int word, int num_bytes, FILE *wav_file);
uint64_t get_time_stamp();
void end_talk_state(struct UserData *data);
int remove_user(void *data, const int key, struct UserData* value);

/* TeamSpeak callbacks */
PLUGINS_EXPORTDLL void ts3plugin_onTalkStatusChangeEvent(uint64 serverConnectionHandlerID, int status, int isReceivedWhisper, anyID clientID);
PLUGINS_EXPORTDLL void ts3plugin_onEditPlaybackVoiceDataEvent(uint64 serverConnectionHandlerID, anyID clientID, short *samples, int sampleCount, int channels);
PLUGINS_EXPORTDLL void ts3plugin_onConnectStatusChangeEvent(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber);

#ifdef __cplusplus
}
#endif

#endif
