#ifndef _LWIPOPTS_EXAMPLE_COMMONH_H
#define _LWIPOPTS_EXAMPLE_COMMONH_H


// Common settings used in most of the pico_w examples
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html for details)

// allow override in some examples
#ifndef NO_SYS
#define NO_SYS                      1 // Desabilita o uso de um sistema operacional (OS) pelo LwIP.
#endif
// allow override in some examples
#ifndef LWIP_SOCKET
#define LWIP_SOCKET                 0 // Desabilita a API de sockets ao estilo BSD.
#endif
#if PICO_CYW43_ARCH_POLL
#define MEM_LIBC_MALLOC             1 // Condicionalmente habilita o uso de malloc e free da biblioteca C padrão para alocação de memória do LwIP.
#else
// MEM_LIBC_MALLOC is incompatible with non polling versions
#define MEM_LIBC_MALLOC             0
#endif
#define MEM_ALIGNMENT               4    // Define o alinhamento de memória para estruturas do LwIP.Neste caso 4 bytes
#define MEM_SIZE                    4000 // Define o tamanho total do heap interno do LwIP (se MEM_LIBC_MALLOC for 0).
#define MEMP_NUM_TCP_SEG            32   // Número máximo de segmentos TCP (partes de dados) que podem ser alocados de uma vez.
#define MEMP_NUM_ARP_QUEUE          10   // Número máximo de pacotes que podem ser enfileirados esperando por uma resposta ARP.
#define PBUF_POOL_SIZE              24   // Número de "pbufs" no pool de alocação de pacotes.
#define LWIP_ARP                    1    // Habilita o Address Resolution Protocol (ARP).
#define LWIP_ETHERNET               1    // Habilita o suporte a quadros Ethernet.
#define LWIP_ICMP                   1    // Habilita o Internet Control Message Protocol (ICMP).
#define LWIP_RAW                    1    // Habilita a API "raw" (bruta) do LwIP.
#define TCP_WND                     (8 * TCP_MSS)  // Define o tamanho da janela de recebimento TCP.
#define TCP_MSS                     1460           // Define o Maximum Segment Size (MSS) para TCP.
#define TCP_SND_BUF                 (8 * TCP_MSS)  // Define o tamanho do buffer de envio TCP.
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS)) //  Define o comprimento máximo da fila de segmentos TCP para envio.
#define LWIP_NETIF_STATUS_CALLBACK  1  // Habilita callbacks para mudanças de status da interface de rede.
#define LWIP_NETIF_LINK_CALLBACK    1  // Habilita callbacks para mudanças no estado do link da interface de rede.
#define LWIP_NETIF_HOSTNAME         1  // Habilita a capacidade de definir um nome de host para a interface de rede.
#define LWIP_NETCONN                0  // Desabilita a API "Netconn" do LwIP.
// Desabilitam várias estatísticas de tempo de execução para memória, sistema, pools de memória e camada de link.
#define MEM_STATS                   0
#define SYS_STATS                   0
#define MEMP_STATS                  0
#define LINK_STATS                  0

#define LWIP_CHKSUM_ALGORITHM       3 // Define qual algoritmo de checksum usar.3 é para uma otimização específica ou uma implementação padrão
#define LWIP_DHCP                   1 // Habilita o Dynamic Host Configuration Protocol (DHCP).
#define LWIP_IPV4                   1 // Habilita o suporte a IPv4.
#define LWIP_TCP                    1 // Habilita o Transmission Control Protocol (TCP).
#define LWIP_UDP                    1 // Habilita o User Datagram Protocol (UDP).
#define LWIP_DNS                    1 // Habilita o Domain Name System (DNS).
#define LWIP_TCP_KEEPALIVE          1 // Habilita a funcionalidade TCP Keep-Alive.
#define LWIP_NETIF_TX_SINGLE_PBUF   1 // Otimização para envio de pacotes.
#define DHCP_DOES_ARP_CHECK         0 // Desabilita a verificação ARP de endereços IP propostos pelo DHCP.
#define LWIP_DHCP_DOES_ACD_CHECK    0 // Desabilita a detecção de conflito de endereço (Address Conflict Detection - ACD) para DHCP.

#ifndef NDEBUG
#define LWIP_DEBUG                  1 // Habilita mensagens de depuração gerais do LwIP.
#define LWIP_STATS                  1 // Habilita a coleta de estatísticas gerais (se as sub-estatísticas individuais estiverem habilitadas).
#define LWIP_STATS_DISPLAY          1 // Habilita a exibição das estatísticas coletadas.
#endif


// Controlam a saída de depuração para módulos específicos do LwIP.
#define ETHARP_DEBUG                LWIP_DBG_OFF
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define PBUF_DEBUG                  LWIP_DBG_OFF
#define API_LIB_DEBUG               LWIP_DBG_OFF
#define API_MSG_DEBUG               LWIP_DBG_OFF
#define SOCKETS_DEBUG               LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define INET_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define RAW_DEBUG                   LWIP_DBG_OFF
#define MEM_DEBUG                   LWIP_DBG_OFF
#define MEMP_DEBUG                  LWIP_DBG_OFF
#define SYS_DEBUG                   LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF
#define TCP_RTO_DEBUG               LWIP_DBG_OFF
#define TCP_CWND_DEBUG              LWIP_DBG_OFF
#define TCP_WND_DEBUG               LWIP_DBG_OFF
#define TCP_FR_DEBUG                LWIP_DBG_OFF
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF
#define TCP_RST_DEBUG               LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF
#define TCPIP_DEBUG                 LWIP_DBG_OFF
#define PPP_DEBUG                   LWIP_DBG_OFF
#define SLIP_DEBUG                  LWIP_DBG_OFF
#define DHCP_DEBUG                  LWIP_DBG_OFF

#endif /* __LWIPOPTS_H__ */
