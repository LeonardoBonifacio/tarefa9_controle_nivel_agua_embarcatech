/*
 * FreeRTOS V202107.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

 #ifndef FREERTOS_CONFIG_H
 #define FREERTOS_CONFIG_H
 
 /*-----------------------------------------------------------
  * Application specific definitions.
  *
  * These definitions should be adjusted for your particular hardware and
  * application requirements.
  *
  * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
  * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
  *
  * See http://www.freertos.org/a00110.html
  *----------------------------------------------------------*/
/* Scheduler Related */
 #define configUSE_PREEMPTION                    1 // Habilita a preempção do escalonador.
 #define configUSE_TICKLESS_IDLE                 0 // Desabilita o modo tickless idle.
 #define configUSE_IDLE_HOOK                     0 // Desabilita a função de hook da tarefa idle.
 #define configUSE_TICK_HOOK                     0 // Desabilita a função de hook do tick do FreeRTOS.
 #define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 ) // Define a frequência do tick do FreeRTOS em Hertz (Hz).
 #define configMAX_PRIORITIES                    32 // Define o número máximo de prioridades que suas tarefas podem ter.
 #define configMINIMAL_STACK_SIZE                ( configSTACK_DEPTH_TYPE ) 256 //  Define o tamanho mínimo da pilha para uma tarefa.
 #define configUSE_16_BIT_TICKS                  0 // Define o tipo de dado para o contador de ticks. Neste caso esta em uint32_t.
 
 #define configIDLE_SHOULD_YIELD                 1 // Permite que a tarefa Idle ceda o processador para outras tarefas de mesma prioridade.
 
 /* Synchronization Related */
 #define configUSE_MUTEXES                       1 // Habilita o uso de mutexes (semáforos binários).
 #define configUSE_RECURSIVE_MUTEXES             1 // Habilita o uso de mutexes recursivos.
 #define configUSE_APPLICATION_TASK_TAG          0 // Desabilita a funcionalidade de "tag" de tarefa de aplicação.
 #define configUSE_COUNTING_SEMAPHORES           1 // Habilita o uso de semáforos contadores.
 #define configQUEUE_REGISTRY_SIZE               8 // Define o tamanho do "Registro de Fila" (Queue Registry), no máximo 8 filas/semáforos.
 #define configUSE_QUEUE_SETS                    1 // Habilita o uso de conjuntos de filas (Queue Sets).
 #define configUSE_TIME_SLICING                  1 // Habilita o time slicing (fatiamento de tempo).
 #define configUSE_NEWLIB_REENTRANT              0 // Desabilita o suporte à reentrância da biblioteca Newlib.
 #define configENABLE_BACKWARD_COMPATIBILITY     0 // Desabilita a compatibilidade com versões antigas do FreeRTOS.
 #define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5 // Define o número de ponteiros de armazenamento local para cada thread/tarefa.
 
 /* System */
 #define configSTACK_DEPTH_TYPE                  uint32_t //  Define o tipo de dados usado para especificar o tamanho da pilha de uma tarefa.
 #define configMESSAGE_BUFFER_LENGTH_TYPE        size_t   //  Define o tipo de dados para o tamanho de buffers de mensagens.
 
 /* Memory allocation related definitions. */
 #define configSUPPORT_STATIC_ALLOCATION         0 // Desabilita a alocação estática de memória.
 #define configSUPPORT_DYNAMIC_ALLOCATION        1 // Habilita a alocação dinâmica de memória.
 #define configTOTAL_HEAP_SIZE                   (128*1024) // Define o tamanho total do heap disponível para alocação dinâmica.
 #define configAPPLICATION_ALLOCATED_HEAP        0 //  Indica se o heap é alocado pela aplicação ou pelo FreeRTOS. Neste caso pelo FreeRTOS
 
 /* Hook function related definitions. */
 #define configCHECK_FOR_STACK_OVERFLOW          0 // Desabilita a verificação de estouro de pilha.
 #define configUSE_MALLOC_FAILED_HOOK            0 // Desabilita a função de hook para falhas de alocação de memória.
 #define configUSE_DAEMON_TASK_STARTUP_HOOK      0 // Desabilita o hook de inicialização da tarefa Daemon (ou Timer Service).
 
 /* Run time and task stats gathering related definitions. */
 #define configGENERATE_RUN_TIME_STATS           0 // Desabilita a geração de estatísticas de tempo de execução.Normamente usada para analise de desempenho
 #define configUSE_TRACE_FACILITY                1 // Habilita a facilidade de rastreamento.O FreeRTOS insere chamadas de rastreamento que podem ser usadas por ferramentas de depuração
 #define configUSE_STATS_FORMATTING_FUNCTIONS    0 // Desabilita as funções de formatação de estatísticas.
 
 /* Co-routine related definitions. */
 #define configUSE_CO_ROUTINES                   0 // Desabilita o uso de co-rotinas.
 #define configMAX_CO_ROUTINE_PRIORITIES         1 // Define o número máximo de prioridades para co-rotinas.
 
 /* Software timer related definitions. */
 #define configUSE_TIMERS                        1 // Habilita o uso de timers de software.
 #define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 ) // Define a prioridade da tarefa de serviço do timer.
 #define configTIMER_QUEUE_LENGTH                10 // Define o tamanho da fila de comando do timer.
 #define configTIMER_TASK_STACK_DEPTH            1024 // Define o tamanho da pilha da tarefa de serviço do timer.
 
 /* Interrupt nesting behaviour configuration. */
 /*
 #define configKERNEL_INTERRUPT_PRIORITY         [dependent of processor]
 #define configMAX_SYSCALL_INTERRUPT_PRIORITY    [dependent on processor and application]
 #define configMAX_API_CALL_INTERRUPT_PRIORITY   [dependent on processor and application]
 */
 
 /* SMP port only */
 #define configNUM_CORES                         1 // Define o número de núcleos que o FreeRTOS pode usar.
 #define configTICK_CORE                         1 // Define qual núcleo gerencia o tick do FreeRTOS.
 #define configRUN_MULTIPLE_PRIORITIES           1 // Permite que o escalonador execute tarefas de diferentes prioridades em paralelo (em um sistema multi-core).
 
 /* RP2040 specific */
 #define configSUPPORT_PICO_SYNC_INTEROP         1 // Habilita a interoperabilidade de sincronização com o SDK do Pico.
 #define configSUPPORT_PICO_TIME_INTEROP         1 // Habilita a interoperabilidade de tempo com o SDK do Pico.
 
 #include <assert.h>
 /* Define to trap errors during development. */
 #define configASSERT(x)                         assert(x) // Define um mecanismo de asserção para depuração.
 
 /* Set the following definitions to 1 to include the API function, or zero
 to exclude the API function. */
 #define INCLUDE_vTaskPrioritySet                1 //  Mudar prioridade de tarefas
 #define INCLUDE_uxTaskPriorityGet               1 // Obter  a prioridade de tarefas
 #define INCLUDE_vTaskDelete                     1 // Deletar um tarefa que não precisa mais ser executada
 #define INCLUDE_vTaskSuspend                    1 // Suspender uma tarefa para ser ativada depois
 #define INCLUDE_vTaskDelayUntil                 1 // Suspende a tarefa até um determinado tempo
 #define INCLUDE_vTaskDelay                      1 // Suspende a tarefa por um determinado tempo
 #define INCLUDE_xTaskGetSchedulerState          1 // Obtém o estado atual dessa tarefa sendo controlada pelo escalonador
 #define INCLUDE_xTaskGetCurrentTaskHandle       1 // Retorna o handle desta tarefa
 #define INCLUDE_uxTaskGetStackHighWaterMark     1 // Retorna a quantidade mínima de espaço de stack que restou para a tarefa em particular desde que ela foi criada.
 #define INCLUDE_xTaskGetIdleTaskHandle          1 // Retorna o handle da tarefa ociosa do FreeRtos
 #define INCLUDE_eTaskGetState                   1 // Retorna o estado atual da tarefa
 #define INCLUDE_xTimerPendFunctionCall          1 // Permite uma função ser executada no contexto do timer do FreeRTOS
 #define INCLUDE_xTaskAbortDelay                 1 // Acorda uma tarefa que está no estado bloqueado
 #define INCLUDE_xTaskGetHandle                  1 // Obtém o handle de uma tarefa pelo seu nome
 #define INCLUDE_xTaskResumeFromISR              1 // Retoma uma tarefa que foi suspensa , usada dentro de ISR
 #define INCLUDE_xQueueGetMutexHolder            1 // Retorna o handle da tarefa que possui o mutex binário
 

 
 #endif /* FREERTOS_CONFIG_H */