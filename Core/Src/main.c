#include "main.h"
#include "can.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include <string.h>
#include <stdio.h>

// CAN 수신 헤더 및 데이터 버퍼
CAN_RxHeaderTypeDef rxHeader;
uint8_t rxData[8];

// 좌표 변수
float x = 0.0f;
float y = 0.0f;

// CAN 수신 여부 확인 플래그
uint8_t dataReceived = 0;

// 조건 비교용 기준 좌표값
float a_3140 = -3.30;
float a_3170f = -10.50;
float a_3180f = -15.50;


float a_chan = 14.00;
float a_kim = 17.50;
float a_u = 23.00;
float a_lee = 33.50;

void SystemClock_Config(void);
void Error_Handler(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_CAN_Init();
    MX_TIM2_Init();

    // CAN 필터 설정: 모든 ID 수신
    CAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_CAN_Start(&hcan);
    HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

    // UART 시작 메시지 전송
    char msg[] = "CAN Started...\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg), HAL_MAX_DELAY);

    // PWM 시작 (TIM2_CH2)
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

    while (1) {
        // 모터 드라이버 STBY 핀 (활성화)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);

        if (dataReceived) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "CAN Received -> X: %.3f, Y: %.3f\r\n", x, y);
            HAL_UART_Transmit(&huart2, (uint8_t*) buffer, strlen(buffer), HAL_MAX_DELAY);
            dataReceived = 0;


        if (x < a_chan) {
            // 전진 방향 설정
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0); // 오른쪽 바퀴 전진
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0); // 왼쪽 바퀴 전진
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 1);

            // 속도 변수 설정
            uint16_t left_speed = 10000;
            uint16_t right_speed = 10000;
            HAL_Delay(100);

            // y 값 기준 좌우 속도 보정
            if (y > 0.01) {
                // 오른쪽으로 쏠림 → 왼쪽 바퀴 느리게
                left_speed = 6000;
                right_speed = 9000;

            } else if (y < -0.01) {
                // 왼쪽으로 쏠림 → 오른쪽 바퀴 느리게
                left_speed = 9000;
                right_speed = 6000;

            } else {
                // 중앙 → 동일 속도
                left_speed = 6000;
                right_speed = 6000;

            }

            // PWM 적용
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, left_speed);   // 왼쪽 바퀴
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, right_speed);  // 오른쪽 바퀴
        }
        else {
            // 도착 지점 도달 시 정지
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 0);
            HAL_Delay(5000);


            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 10000);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 10000);


            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 1);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 1);
            HAL_Delay(14500);

            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);

        }
       }

        //////////////////////////////////////////////////////return////////////////////
        if (x > 0) {
            // 전진 방향 설정
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0); // 오른쪽 바퀴 전진
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 1);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0); // 왼쪽 바퀴 전진
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 1);

            // 속도 변수 설정
            uint16_t left_speed = 10000;
            uint16_t right_speed = 10000;
            HAL_Delay(100);

            // y 값 기준 좌우 속도 보정
            if (y < 0.01) {
                // 오른쪽으로 쏠림 → 왼쪽 바퀴 느리게
                left_speed = 6000;
                right_speed = 9000;

            } else if (y > -0.01) {
                // 왼쪽으로 쏠림 → 오른쪽 바퀴 느리게
                left_speed = 9000;
                right_speed = 6000;

            } else {
                // 중앙 → 동일 속도
                left_speed = 6000;
                right_speed = 6000;

            }

            // PWM 적용
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, left_speed);   // 왼쪽 바퀴
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, right_speed);  // 오른쪽 바퀴
        } else {
            // 도착 지점 도달 시 정지
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 0);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 0);
            HAL_Delay(5000);


            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 10000);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 10000);


            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, 1);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, 0);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, 0);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, 1);
            HAL_Delay(14500);

            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);

        }


       HAL_Delay(100);  // 0.1초 주기

        }

}

// CAN 수신 콜백
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxData);

    // x, y 좌표 데이터 파싱
    int16_t rawX = (rxData[0] << 8) | rxData[1];
    int16_t rawY = (rxData[2] << 8) | rxData[3];

    x = rawX / 1000.0f;
    y = rawY / 1000.0f;

    dataReceived = 1;
}

// 시스템 클럭 설정
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

// 에러 핸들러
void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
