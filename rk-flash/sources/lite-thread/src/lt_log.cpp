/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   28/09/2025
 ******************************************************************************
**/

#include "lt_log.h"
#include "lt_task.h"

#define OFFSET_DEFAULT_FILE_LOG         (0x00)

static pthread_mutex_t lt_log_fatal_mt = PTHREAD_MUTEX_INITIALIZER;
static log_fatal_t lt_log_fatal_object[LT_LOG_FATAL_OBJECT_MAX_SIZE];
static uint32_t lt_log_fatal_block_used = 0;
static uint32_t lt_log_fatal_index = 0;

static void fatal_log_dump_file();
void fatal_log_dump();

#ifdef _WIN32
    #define localtime_r(timep, result) localtime_s(result, timep)
#endif

/*****************************************************************************
 * log operation file
 *****************************************************************************/
static void log_read(uint32_t address, uint8_t* data, uint32_t len) {
    FILE* f = fopen(LT_FATAL_LOG_BIN_FILE_PATH, "rb");
    if (f) {
        fseek(f, address, SEEK_SET);
        size_t result = fread(data, len, 1, f);
        if (result != 1) {
            memset(data, 0, len);
        }
        fclose(f);
    }
    else {
        memset(data, 0, len);
    }
}

static void log_write(uint32_t address, uint8_t* data, uint32_t len) {
    FILE* f = fopen(LT_FATAL_LOG_BIN_FILE_PATH, "r+b");
    if (!f) {
        f = fopen(LT_FATAL_LOG_BIN_FILE_PATH, "w+b");
    }
    if (f) {
        fseek(f, address, SEEK_SET);
        fwrite(data, len, 1, f);
        fflush(f);
        fclose(f);
    }
}

/*****************************************************************************
 * fatal log service
 *****************************************************************************/
void fatal_log_init() {
    pthread_mutex_lock(&lt_log_fatal_mt);

    memset(lt_log_fatal_object, 0, sizeof(lt_log_fatal_object));
    log_read(OFFSET_DEFAULT_FILE_LOG, (uint8_t*)&lt_log_fatal_block_used, sizeof(lt_log_fatal_block_used));
    log_read(OFFSET_DEFAULT_FILE_LOG + sizeof(lt_log_fatal_block_used), (uint8_t*)&lt_log_fatal_index, sizeof(lt_log_fatal_index));
    log_read(OFFSET_DEFAULT_FILE_LOG + sizeof(lt_log_fatal_block_used) + sizeof(lt_log_fatal_index), (uint8_t*)&lt_log_fatal_object, sizeof(lt_log_fatal_object));

    pthread_mutex_unlock(&lt_log_fatal_mt);

    LT_LOG_KERNEL("[fatal_log] fatal log initialized successfully\n");
}

void fatal_log_dbg(const char* msg, uint8_t code) {
    pthread_mutex_lock(&lt_log_fatal_mt);

    /* fatal log info */
    log_fatal_t* log = &lt_log_fatal_object[lt_log_fatal_index];
    memset(log, 0, sizeof(log_fatal_t));
    strncpy(log->msg, msg, sizeof(log->msg) - 1);
    log->code = code;
    clock_gettime(CLOCK_REALTIME, &log->ts);
    log->task_id = get_current_task_id();

    /* fatal log update */
    lt_log_fatal_index = (lt_log_fatal_index + 1) % LT_LOG_FATAL_OBJECT_MAX_SIZE;
    if (lt_log_fatal_block_used < LT_LOG_FATAL_OBJECT_MAX_SIZE) {
        lt_log_fatal_block_used++;
    }

    log_write(OFFSET_DEFAULT_FILE_LOG, (uint8_t*)&lt_log_fatal_block_used, sizeof(lt_log_fatal_block_used));
    log_write(OFFSET_DEFAULT_FILE_LOG + sizeof(lt_log_fatal_block_used), (uint8_t*)&lt_log_fatal_index, sizeof(lt_log_fatal_index));
    log_write(OFFSET_DEFAULT_FILE_LOG + sizeof(lt_log_fatal_block_used) + sizeof(lt_log_fatal_index), (uint8_t*)&lt_log_fatal_object, sizeof(lt_log_fatal_object));

    /* fatal log dump */
    fatal_log_dump();
    fatal_log_dump_file();

    pthread_mutex_unlock(&lt_log_fatal_mt);
}

void fatal_log_clear() {
    pthread_mutex_lock(&lt_log_fatal_mt);

    memset(lt_log_fatal_object, 0, sizeof(lt_log_fatal_object));
    lt_log_fatal_index = 0;
    lt_log_fatal_block_used = 0;
    log_write(OFFSET_DEFAULT_FILE_LOG, (uint8_t*)&lt_log_fatal_block_used, sizeof(lt_log_fatal_block_used));
    log_write(OFFSET_DEFAULT_FILE_LOG + sizeof(lt_log_fatal_block_used), (uint8_t*)&lt_log_fatal_index, sizeof(lt_log_fatal_index));
    log_write(OFFSET_DEFAULT_FILE_LOG + sizeof(lt_log_fatal_block_used) + sizeof(lt_log_fatal_index), (uint8_t*)&lt_log_fatal_object, sizeof(lt_log_fatal_object));

    pthread_mutex_unlock(&lt_log_fatal_mt);

    LT_LOG("[fatal_log] fatal log erased successfully\n");
}

void fatal_log_dump() {
    uint32_t fatal_log_counter = 0;
    log_read(OFFSET_DEFAULT_FILE_LOG, (uint8_t*)&fatal_log_counter, sizeof(fatal_log_counter));
    log_read(OFFSET_DEFAULT_FILE_LOG + sizeof(lt_log_fatal_block_used) + sizeof(lt_log_fatal_index), (uint8_t*)&lt_log_fatal_object, sizeof(lt_log_fatal_object));
    
    LT_LOG("fatal log information\n");
    LT_LOG("fatal log trace: %d\n", fatal_log_counter);
    for (uint32_t i = 0; i < fatal_log_counter; i++) {
        log_fatal_t* log = &lt_log_fatal_object[i];
        char time_buf[64];
        struct tm tm_info;
        time_t seconds = log->ts.tv_sec;
        localtime_r(&seconds, &tm_info);
        strftime(time_buf, sizeof(time_buf), "%d/%m/%Y %H:%M:%S", &tm_info);

        LT_LOG("[%s] [index]: %04d [task_id]: %02X [code]: 0x%02X [fatal_type]: %s\n", time_buf, i, log->task_id, log->code, log->msg);
    }
}

void fatal_log_dump_file() {
    uint32_t fatal_log_counter = 0;
    log_read(OFFSET_DEFAULT_FILE_LOG, (uint8_t*)&fatal_log_counter, sizeof(fatal_log_counter));
    log_read(OFFSET_DEFAULT_FILE_LOG + sizeof(lt_log_fatal_block_used) + sizeof(lt_log_fatal_index), (uint8_t*)&lt_log_fatal_object, sizeof(lt_log_fatal_object));

    FILE* f = fopen(LT_FATAL_LOG_TEXT_FILE_PATH, "w");
    if (f) {
        for (uint32_t i = 0; i < fatal_log_counter; i++) {
            log_fatal_t* log = &lt_log_fatal_object[i];
            char time_buf[64];
            struct tm tm_info;
            time_t seconds = log->ts.tv_sec;
            localtime_r(&seconds, &tm_info);
            strftime(time_buf, sizeof(time_buf), "%d/%m/%Y %H:%M:%S", &tm_info);

            fprintf(f, "[%s] [index]: %04d [task_id]: %02X [code]: 0x%02X [fatal_type]: %s\n", time_buf, i, log->task_id, log->code, log->msg);
        }
        fflush(f);
        fclose(f);
    }
}
