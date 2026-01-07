#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>

#include "freertos/freeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "rom/ets_sys.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"

#define PIN_CLK 13
#define PIN_A 12
#define PIN_LED 2

#define PIN_BITMASK ((1ULL << PIN_CLK) | \
                     (1ULL << PIN_A) |   \
                     (1ULL << PIN_LED))

// int lampIndex[] = {23, 5, 17, 3, -1, 13, 12, -1, 22, 21, 18, 8, 15, 4, 11, 10, 14, 16, 20, 19, 10};
int lampIndex[] = {23, 5, 17, 3, -1, 13, 12, -1, 22, 21, 18, 8, 15, 4, 11, 10, 14, 16, 20, 19, 10};
int lampVals[20];

int receivingAnim = 0;
int frameIndex = 0;

struct Animation
{
    char *name;
    int numFrames;
    int fps;
    int **frames;
    int numlamps;
} anim; // COPIED FROM LAMP PROGRAMMER CODE UPDATE THERE IF UPDATING
int animPlay = 0;

void processLine(const char *line, int lineLen)
{

    if (!lineLen)
    {
        printf("no line\n");
    }

    int numTokens = 0;
    char **tokens = NULL;

    const char *tokenStart = line;

    for (int i = 0; i <= lineLen; i++)
    {

        if ((i == lineLen) || (line[i] == ' '))
        {
            int tokenLen = line + i - tokenStart;

            if (tokenLen > 0)
            {
                numTokens++;
                tokens = realloc(tokens, sizeof(char *) * numTokens);

                tokens[numTokens - 1] = malloc(tokenLen + 1);

                memcpy(tokens[numTokens - 1], tokenStart, tokenLen);

                tokens[numTokens - 1][tokenLen] = '\0';
            }

            tokenStart = line + i + 1;
        }
    }

    if (!numTokens)
    {
        printf("no tokens\n");
        return;
    }

    if (strcmp(tokens[0], "led") == 0)
    {
        if (strcmp(tokens[1], "on") == 0)
        {
            gpio_set_level(PIN_LED, 1);
        }
        if (strcmp(tokens[1], "off") == 0)
        {
            gpio_set_level(PIN_LED, 0);
        }
    }

    if (strcmp(tokens[0], "lamp") == 0)
    {

        int lamp = atoi(tokens[1]);

        if (strcmp(tokens[2], "brightness") == 0)
        {
            int brightness = atoi(tokens[3]);
            lampVals[lamp] = brightness;
        }
        if (strcmp(tokens[2], "face") == 0)
        {
            int faceIndex = atoi(tokens[3]);
            lampIndex[lamp] = faceIndex;
        }
    }

    if (strcmp(tokens[0], "ANIM_END") == 0)
    {
        receivingAnim = 0;
        frameIndex = 0;

        printf("recived: %s, with %i frames, of %i lamps, and an fps of %i\n",
               anim.name,
               anim.numFrames,
               anim.numlamps,
               anim.fps);

        // printf("reciced frame data:\n");
        // for (int i = 0; i < anim.numFrames; i++)
        // {
        //     printf("frame: %i\n", i);
        //     for (int j = 0; j < anim.numlamps; j++)
        //     {
        //         printf("%i ", anim.frames[i][j]);
        //     }
        //     printf("\n");
        // }
    }

    if (receivingAnim)
    {

        if (strcmp(tokens[0], "META") == 0)
        {
            free(anim.name);
            int nameLen = strlen(tokens[2]) + 1;
            anim.name = malloc(nameLen);
            memcpy(anim.name, tokens[2], nameLen);

            anim.fps = atoi(tokens[4]);
            anim.numFrames = atoi(tokens[6]);
            anim.numlamps = atoi(tokens[8]);

            anim.frames = malloc(sizeof(int *) * anim.numFrames);
            for (int i = 0; i < anim.numFrames; i++)
            {
                anim.frames[i] = malloc(sizeof(int) * anim.numlamps);
            }
        }
        else
        {

            for (int i = 0; i < anim.numlamps; i++)
            {
                anim.frames[frameIndex][i] = atoi(tokens[i]);
            }
            frameIndex++;
        }
    }

    if (strcmp(tokens[0], "SENDING_ANIM") == 0)
    {
        receivingAnim = 1;
        frameIndex = 0;
    }

    if (strcmp(tokens[0], "ANIM_STOP") == 0)
    {
        animPlay = 0;
    }
    if (strcmp(tokens[0], "ANIM_PLAY") == 0)
    {
        animPlay = 1;
    }

    for (int i = 0; i < numTokens; i++)
    {
        free(tokens[i]);
    }
    free(tokens);
}

void app_main(void)
{
    printf("LampProgrammer\n");

    gpio_config_t io_config = {};
    io_config.pin_bit_mask = PIN_BITMASK;
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.pull_up_en = 0;
    io_config.pull_down_en = 0;
    gpio_config(&io_config);

    char *resp = malloc(128);
    int lineLen = 0;

    memset(&anim, 0x0, sizeof(struct Animation));
    memset(lampVals, 0x0, sizeof(lampVals));
    for (int i = 0; i < 20; i++)
    {
        lampVals[i] = 0;
    }

    int frame = 0;
    int pwmCnt = 0;
    int pwmDepth = 16;

    while (1)
    {

        vTaskDelay(1);
        int c;
        if ((c = fgetc(stdin)) != EOF)
        {

            if (c == '\n')
            {
                resp[lineLen] = '\0';
                printf("esp response = %s\n", resp);

                processLine(resp, lineLen);

                memset(resp, 0x0, 128);
                lineLen = 0;
            }
            else
            {
                resp[lineLen] = c;
                lineLen++;
            }
        }

        if (receivingAnim)
        {
            continue;
        }

        int *frameBuff = NULL;
        int fps = 0;

        if (animPlay)
        {
            frameBuff = anim.frames[frame];
            fps = anim.fps;
        }
        else
        {
            frameBuff = lampVals;
            fps = 60;
        }

        unsigned long startTime = esp_timer_get_time();
        unsigned long usPerFrame = 1000000 / fps;
        int index = 0;
        while (esp_timer_get_time() < (startTime + usPerFrame))
        {
            pwmCnt = (pwmCnt++ == pwmDepth) ? 0 : pwmCnt;
            for (int i = 0; i < 24; i++)
            {
                int pinVal = 0;
                if (i > 3)
                {
                    index = i - 4;
                    pinVal = frameBuff[index] > pwmCnt;
                }

                gpio_set_level(PIN_A, pinVal);
                gpio_set_level(PIN_CLK, 1);
                usleep(1);
                gpio_set_level(PIN_CLK, 0);
                usleep(1);
            }
            usleep(500);
        }
        frame++;
        if (frame >= anim.numFrames)
        {
            frame = 0;
        }
    }
}
