/* Header fuer TNT-Runprogramme */

/* Eincompilierter Pfad fuer INI Dateien*/
#define TNT_INIT_PATH "/usr/local/share/tnt/conf"

/* Eincompilierter Run-Pfad, wird zB vom Runprogramm "hilfe" benoetigt */
#define TNT_RUN_PATH "/usr/local/libexec/tnt"

/* Eincompilierter Main-Pfad, wird zB vom Logbuchprogramm benoetigt */
#define TNT_MAIN_PATH "/var/log"

/* Makrodefinitionen fuer Enviromentvariablen */
#define GET_CALLSIGN (char *) getenv("CALLSIGN")
#define GET_LOGNAME (char *) getenv("LOGNAME")
#define GET_CALLSSID (char *) getenv("CALLSSID")

