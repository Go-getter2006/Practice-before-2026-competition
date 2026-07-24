/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Koji KITAYAMA
 * Copyright (c) 2024, Brent Kowal (Analog Devices, Inc)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file is part of the TinyUSB stack.
 */

#include "tusb_option.h"

// #if CFG_TUD_ENABLED && defined(TUP_USBIP_MUSB)

#define MUSB_DEBUG 2
#define MUSB_REGS(rhport)   ((musb_regs_t*) MUSB_BASES[rhport])

#if __GNUC__ > 8 && defined(__ARM_FEATURE_UNALIGNED)
/* GCC warns that an address may be unaligned, even though
 * the target CPU has the capability for unaligned memory access. */
_Pragma("GCC diagnostic ignored \"-Waddress-of-packed-member\"");
#endif

#include "dcd_musbfsfc.h"
#include "device/dcd.h"

// Following symbols must be defined by port header
// - musb_dcd_int_enable/disable/clear/get_enable
// - musb_dcd_int_handler_enter/exit
#if defined(TUP_USBIP_MUSB_TI)
  #include "musb_ti.h"
#else
  #error "Unsupported MCU"
#endif

/*------------------------------------------------------------------
 * MACRO TYPEDEF CONSTANT ENUM DECLARATION
 *------------------------------------------------------------------*/

#define REQUEST_TYPE_INVALID  (0xFFu)

typedef union {
  volatile uint8_t   u8;
  volatile uint16_t  u16;
  volatile uint32_t  u32;
} hw_fifo_t;

typedef struct TU_ATTR_PACKED
{
  void      *buf;      /* the start address of a transfer data buffer */
  uint16_t  length;    /* the number of bytes in the buffer */
  uint16_t  remaining; /* the number of bytes remaining in the buffer */
} pipe_state_t;

typedef struct
{
  TU_ATTR_ALIGNED(4) tusb_control_request_t setup_packet;
  uint16_t     remaining_ctrl; /* The number of bytes remaining in data stage of control transfer. */
  int8_t       status_out;
  pipe_state_t pipe0;
  pipe_state_t pipe[2][TUP_DCD_ENDPOINT_MAX-1];   /* pipe[direction][endpoint number - 1] */
  uint16_t     pipe_buf_is_fifo[2]; /* Bitmap. Each bit means whether 1:TU_FIFO or 0:POD. */
} dcd_data_t;

static dcd_data_t _dcd;


//--------------------------------------------------------------------
// HW FIFO Helper
// Note: Index register is already set by caller
//--------------------------------------------------------------------

TU_ATTR_ALWAYS_INLINE static inline void hwfifo_reset(musb_regs_t* musb, unsigned epnum, unsigned is_rx) {
  (void) musb; (void) epnum; (void) is_rx;
  // nothing to do for static FIFO
}

TU_ATTR_ALWAYS_INLINE static inline bool hwfifo_config(musb_regs_t* musb, unsigned epnum, unsigned is_rx, unsigned mps,
                                                       bool double_packet) {
  (void) epnum;
  (void) mps;
  (void) is_rx;
  (void) double_packet;
  (void) musb;
  // nothing to do for static FIFO
  return true;
}

/*
 * @brief      Flushes previously selected endpoint FIFO
 *
 * @param[in] musb: Pointer to USB registers
 *            epnum: Endpoint number
 *            is_rx: Flag to indicate if endpoint is rx or tx
 *            clear_dtog: Flag to force toggle data
 * 
 * @retval  void
 */
TU_ATTR_ALWAYS_INLINE static inline void hwfifo_flush(musb_regs_t* musb, unsigned epnum, unsigned is_rx, bool clear_dtog) {
  (void) epnum;
  musb_ep_csr_t* ep_csr = &musb->indexed_csr;

  /*
   * If clear data toggle is true then set variable to bit
   * with respect to endpoint direction
   */
  const uint8_t csrl_dtog = clear_dtog ? MUSB_CSR1_CLEAR_DATA_TOGGLE(is_rx) : 0;
  const uint8_t flush_fifo = MUSB_CSR1_FLUSH_FIFO(is_rx);
  /* If packet tx or rx packet ready is high then flush the FIFO */
  if (is_rx) {
    for (uint8_t i = 0; i < 2; i++) {
      if (ep_csr->rx_csr1 & MUSB_CSR1_PACKET_READY(is_rx)) {
        ep_csr->rx_csr1 = flush_fifo | csrl_dtog;
      }
    }
  } else {
    for (uint8_t i = 0; i < 2; i++) {
      if (ep_csr->tx_csr1 & MUSB_CSR1_PACKET_READY(is_rx)) {
        ep_csr->tx_csr1 = flush_fifo | csrl_dtog;
      }
    }
  }
}

/*
 * @brief      Handles writing to USB FIFO0
 *
 * @param[in] rhport:  Roothub instance being utilized
 * @param[in] ep_addr: Endpoint address
 * 
 * @retval  true : success
 */
static void pipe_write_packet(void *buf, volatile void *fifo, unsigned len)
{
  volatile hw_fifo_t *reg = (volatile hw_fifo_t*)fifo;
  uint8_t *addr = (uint8_t*)buf;

  while (len--) {
    reg->u8 = *(uint8_t const *)addr++;
  }
}

static void pipe_read_packet(void *buf, volatile void *fifo, unsigned len)
{
  volatile hw_fifo_t *reg = (volatile hw_fifo_t*)fifo;
  uint8_t *addr = (uint8_t*)buf;

  while (len--) {
    *addr++ = reg->u8;
  }
}

//Requires more investigation but leaving it as is for now
static void pipe_read_write_packet_ff(tu_fifo_t *f, volatile void *fifo, unsigned len, unsigned dir)
{
  static const struct {
    void (*tu_fifo_get_info)(tu_fifo_t *f, tu_fifo_buffer_info_t *info);
    void (*tu_fifo_advance)(tu_fifo_t *f, uint16_t n);
    void (*pipe_read_write)(void *buf, volatile void *fifo, unsigned len);
  } ops[] = {
    /* OUT */ {tu_fifo_get_write_info,tu_fifo_advance_write_pointer,pipe_read_packet},
    /* IN  */ {tu_fifo_get_read_info, tu_fifo_advance_read_pointer, pipe_write_packet},
  };
  tu_fifo_buffer_info_t info;
  ops[dir].tu_fifo_get_info(f, &info);
  unsigned total_len = len;
  len = TU_MIN(total_len, info.len_lin);
  ops[dir].pipe_read_write(info.ptr_lin, fifo, len);
  unsigned rem = total_len - len;
  if (rem) {
    len = TU_MIN(rem, info.len_wrap);
    ops[dir].pipe_read_write(info.ptr_wrap, fifo, len);
    rem -= len;
  }
  ops[dir].tu_fifo_advance(f, total_len - rem);
}

/*
 * @brief      Handles endpoint n transfer
 *
 * @param[in] rhport:  Roothub instance being utilized
 *            ep_addr: Endpoint address
 *            buffer: buffer pointer
 *            total_bytes: total bytes present
 * 
 * @retval  true : success
 */
static void process_setup_packet(uint8_t rhport) {
  musb_regs_t* musb_regs = MUSB_REGS(rhport);

  /* Pointer to setup packet member */
  uint8_t *p = (void*)&_dcd.setup_packet;
  /* Pointer to EP0 FIFO */
  volatile uint8_t *fifo_ptr = &musb_regs->fifo[0];
  /* Setup packet is a total of 64-bits, so read the FIFO 8 times */
  for (uint8_t i = 0; i < 8; i++) {
    p[i] = *fifo_ptr;
  }

  /* Initialize EP0 status and notify stack of setup packet */
  _dcd.pipe0.buf       = NULL;
  _dcd.pipe0.length    = 0;
  _dcd.pipe0.remaining = 0;
  dcd_event_setup_received(rhport, (const uint8_t*)(uintptr_t)&_dcd.setup_packet, true);

  /* Set length and remainder as length reported in setup packet */
  const unsigned len    = _dcd.setup_packet.wLength;
  _dcd.remaining_ctrl   = len;

  /* Get direction of EP0 */
  const unsigned dir_in = tu_edpt_dir(_dcd.setup_packet.bmRequestType);

  /* Clear RX FIFO and reverse the transaction direction */
  if (len && dir_in) {
    musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, 0);
    ep_csr->csr0 = MUSB_CSR0_RXPKTRDYC;
  }
}

/*
 * @brief      Handles endpoint n tx transfer
 *
 * @param[in] rhport:  Roothub instance being utilized
 *            ep_addr: Endpoint address
 * 
 * @retval  true : success
 */
static bool handle_xfer_in(uint8_t rhport, uint_fast8_t ep_addr)
{
  /* EP number based off EP address */
  unsigned ep_num = tu_edpt_number(ep_addr);
  /* EP number starting from 0 */
  unsigned epnum_minus1 = ep_num - 1;
  /* Pointer to struct containing current EP data */
  pipe_state_t  *pipe = &_dcd.pipe[tu_edpt_dir(ep_addr)][epnum_minus1];
  /* Tracking variable for remaining data in EP */
  const unsigned rem  = pipe->remaining;

  /* If there is nothing remaining then reset the buffer and return */
  if (!rem) {
    pipe->buf = NULL;
    return true;
  }

  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, ep_num);

  /* Grab what is the current max packet size of the endpoint */
  const unsigned max_packet_size = ep_csr->tx_max_ep;
  /* Determine which parameter to set len to */
  const unsigned len = TU_MIN(max_packet_size, rem);
  /* Pointer to EP buffer */
  void          *buf = pipe->buf;
  /* Pointer to EP FIFO */
  volatile void *fifo_ptr = &musb_regs->fifo[ep_num];

  // TU_LOG1("   %p mps %d len %d rem %d\r\n", buf, mps, len, rem);

  if (len) {
    if (_dcd.pipe_buf_is_fifo[TUSB_DIR_IN] & TU_BIT(epnum_minus1)) {
      pipe_read_write_packet_ff(buf, fifo_ptr, len, TUSB_DIR_IN);
    } else {
      pipe_write_packet(buf, fifo_ptr, len);
      pipe->buf       = buf + len;
    }
    pipe->remaining = rem - len;
  }
  /* Set CSR of EP to notify that TX packet is ready */
  ep_csr->tx_csr1 = MUSB_TXCSRL1_TXRDY;
  
  // TU_LOG1(" TXCSRL%d = %x %d\r\n", epnum, ep_csr->tx_csrl, rem - len);
  
  return false;
}

/*
 * @brief      Handles endpoint n rx transfer
 *
 * @param[in] rhport:  Roothub instance being utilized
 *            ep_addr: Endpoint address
 * 
 * @retval  true : success
 */
static bool handle_xfer_out(uint8_t rhport, uint_fast8_t ep_addr)
{
  unsigned epnum = tu_edpt_number(ep_addr);
  unsigned epnum_minus1 = epnum - 1;
  pipe_state_t  *pipe = &_dcd.pipe[tu_edpt_dir(ep_addr)][epnum_minus1];
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, epnum);

  // TU_LOG1(" RXCSRL%d = %x\r\n", epnum_minus1 + 1, ep_csr->rx_csr1);

  /* Assert to confirm that something was received */
  TU_ASSERT(ep_csr->rx_csr1 & MUSB_RXCSRL1_RXRDY);

  const unsigned mps = ep_csr->rx_max_ep;
  const unsigned rem = pipe->remaining;
  const unsigned vld = ((ep_csr->rx_count_upper << 8) | 
                         ep_csr->rx_count_lower);
  const unsigned len = TU_MIN(TU_MIN(rem, mps), vld);
  void          *buf = pipe->buf;
  volatile void *fifo_ptr = &musb_regs->fifo[epnum];

  if (len) {
    if (_dcd.pipe_buf_is_fifo[TUSB_DIR_OUT] & TU_BIT(epnum_minus1)) {
      pipe_read_write_packet_ff(buf, fifo_ptr, len, TUSB_DIR_OUT);
    } else {
      pipe_read_packet(buf, fifo_ptr, len);
      pipe->buf       = buf + len;
    }
    pipe->remaining = rem - len;
  }
  if ((len < mps) || (rem == len)) {
    pipe->buf = NULL;
    return NULL != buf;
  }
  ep_csr->rx_csr1 = 0; /* Clear RXRDY bit */
  return false;
}

/*
 * @brief      Handles endpoint n transfer
 *
 * @param[in] rhport:  Roothub instance being utilized
 *            ep_addr: Endpoint address
 *            buffer: buffer pointer
 *            total_bytes: total bytes present
 * 
 * @retval  true : success
 */
static bool edpt_n_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes)
{
  unsigned ep_num = tu_edpt_number(ep_addr);
  unsigned epnum_minus1 = ep_num - 1;
  unsigned dir_in       = tu_edpt_dir(ep_addr);

  /* Initialize endpoint */
  pipe_state_t *pipe = &_dcd.pipe[dir_in][epnum_minus1];
  pipe->buf          = buffer;
  pipe->length       = total_bytes;
  pipe->remaining    = total_bytes;

  /* 
   * If TX, handle it with handle_xfer_in(), otherwise if RX
   * then clear the rx_packet_ready bit
   */
  if (dir_in) {
    handle_xfer_in(rhport, ep_addr);
  } else {
    musb_regs_t* musb_regs = MUSB_REGS(rhport);
    musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, ep_num);
    if (ep_csr->rx_csr1 & MUSB_RXCSRL1_RXRDY) ep_csr->rx_csr1 = 0;
  }
  return true;
}

/*
 * @brief      Handles endpoint 0 transfers
 *
 * @param[in] rhport:  Roothub instance being utilized
 *            ep_addr: Endpoint address
 *            buffer: buffer pointer
 *            total_bytes: total bytes present
 * 
 * @retval  true : success
 */
static bool edpt0_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t *buffer, uint16_t total_bytes)
{
  (void)rhport;
  TU_ASSERT(total_bytes <= 64); /* Current implementation supports for only up to 64 bytes. */
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, 0);
  const unsigned req = _dcd.setup_packet.bmRequestType;
  TU_ASSERT(req != REQUEST_TYPE_INVALID || total_bytes == 0);

  if (req == REQUEST_TYPE_INVALID || _dcd.status_out) {
    /*
     * STATUS OUT stage.
     * MUSB controller automatically handles STATUS OUT packets without
     * software helps. We do not have to do anything. And STATUS stage
     * may have already finished and received the next setup packet
     * without calling this function, so we have no choice but to
     * invoke the callback function of status packet here. 
     */
    // TU_LOG1(" STATUS OUT ep_csr->csr0 = %x\r\n", ep_csr->csr0);
    _dcd.status_out = 0;
    if (req == REQUEST_TYPE_INVALID) {
      dcd_event_xfer_complete(rhport, ep_addr, total_bytes, XFER_RESULT_SUCCESS, false);
    } else {
      /*
       * The next setup packet has already been received, it aborts
       * invoking callback function to avoid confusing TUSB stack.
       */
      TU_LOG1("Drop CONTROL_STAGE_ACK\r\n");
    }
    return true;
  }

  /* Grab direction of EP0 */
  const unsigned dir_in = tu_edpt_dir(ep_addr);
  /* Check direction of EP0 at time of setup packet request */
  if (tu_edpt_dir(req) == dir_in) { /* DATA stage */
    /* 
     * Check that the total bytes is less than or equal
     * to remaining control bytes in data stage
     */
    TU_ASSERT(total_bytes <= _dcd.remaining_ctrl);
    const unsigned rem = _dcd.remaining_ctrl;
    const unsigned len = TU_MIN(TU_MIN(rem, 64), total_bytes);
    volatile void *fifo_ptr = &musb_regs->fifo[0];

    if (dir_in) { /* In data phase for control transfer */
      pipe_write_packet(buffer, fifo_ptr, len);

      /* Update EP0 status */
      _dcd.pipe0.buf       = buffer + len;
      _dcd.pipe0.length    = len;
      _dcd.pipe0.remaining = 0;
      _dcd.remaining_ctrl  = rem - len;

      if ((len < 64) || (rem == len)) {
        /* If last packet change to STATUS/SETUP stage */
        _dcd.setup_packet.bmRequestType = REQUEST_TYPE_INVALID;
        _dcd.status_out = 1;
        /* Flush TX FIFO and reverse the transaction direction. */
        ep_csr->csr0 = MUSB_CSR0_TXRDY | MUSB_CSR0_DATAEND;
      } else {
        /* 
         * If it is not the last packet then set InPktRdy to
         * flush TX FIFO and return ACK
         */
        ep_csr->csr0 = MUSB_CSR0_TXRDY;
      }
      // TU_LOG1(" IN ep_csr->csr0 = %x\r\n", ep_csr->csr0);
    } else { /* Out data phase for control transfer */
      // TU_LOG1(" OUT ep_csr->csr0 = %x\r\n", ep_csr->csr0);
      _dcd.pipe0.buf       = buffer;
      _dcd.pipe0.length    = len;
      _dcd.pipe0.remaining = len;
      /* 
       * If it is not last packet then set ServicedOutPktRdy bit
       * to clear RX FIFO and return ACK
       */
      ep_csr->csr0 = MUSB_CSR0_RXPKTRDYC;
    }
  } else if (dir_in) {
    // TU_LOG1(" STATUS IN ep_csr->csr0l  = %x\r\n", ep_csr->csr0l);
    _dcd.pipe0.buf = NULL;
    _dcd.pipe0.length    = 0;
    _dcd.pipe0.remaining = 0;
    /* Clear RX FIFO and reverse the transaction direction */
    ep_csr->csr0 = MUSB_CSR0_RXPKTRDYC | MUSB_CSR0_DATAEND;
  }
  return true;
}

/*
 * @brief      Process endpoint 0 states
 *
 * @param[in] rhport:  Roothub instance being utilized
 * 
 * @retval  none
 */
static void process_ep0(uint8_t rhport)
{
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, 0);
  uint_fast8_t csr_zero = ep_csr->csr0;

  // TU_LOG1(" EP0 ep_csr->csr0l = %x\r\n", csrl);
  // 21.1.5: endpoint 0 service routine as peripheral

  /* Check if Sent Stall bit is active and clear the bit */
  if (csr_zero & MUSB_CSR0_SENT_STALLED) {
    /*
     * Returned STALL packet to HOST and
     * state will go to IDLE 
     */
    ep_csr->csr0 = 0;
    return;
  }

  /* 
   * Check if setup end bit is set, if set enable
   * ServicedSetupEnd bit
   */
  unsigned req = _dcd.setup_packet.bmRequestType;
  if (csr_zero & MUSB_CSR0_SETEND) {
    TU_LOG1("   ABORT by the next packets\r\n");
    ep_csr->csr0 = MUSB_CSR0_SETENDC;
    if (req != REQUEST_TYPE_INVALID && _dcd.pipe0.buf) {
      /* DATA stage was aborted by receiving STATUS or SETUP packet. */
      _dcd.pipe0.buf = NULL;
      _dcd.setup_packet.bmRequestType = REQUEST_TYPE_INVALID;
      dcd_event_xfer_complete(rhport,
                              req & TUSB_DIR_IN_MASK,
                              _dcd.pipe0.length - _dcd.pipe0.remaining,
                              XFER_RESULT_SUCCESS, true);
    }
    req = REQUEST_TYPE_INVALID;
    /* Received SETUP packet */
    if (!(csr_zero & MUSB_CSR0_RXRDY)) return;
  }

  /* If RX then follow flowchart in section 6.5.3 */
  if (csr_zero & MUSB_CSR0_RXRDY) {
    /* Received SETUP or DATA OUT packet */
    if (req == REQUEST_TYPE_INVALID) {
      /* SETUP */
      TU_ASSERT(sizeof(tusb_control_request_t) == ep_csr->count0,);
      process_setup_packet(rhport);
      return;
    }
    if (_dcd.pipe0.buf) {
      /* DATA OUT */
      const unsigned vld = ep_csr->count0;
      const unsigned rem = _dcd.pipe0.remaining;
      const unsigned len = TU_MIN(TU_MIN(rem, 64), vld);
      volatile void *fifo_ptr = &musb_regs->fifo[0];
      pipe_read_packet(_dcd.pipe0.buf, fifo_ptr, len);

      _dcd.pipe0.remaining = rem - len;
      _dcd.remaining_ctrl -= len;

      _dcd.pipe0.buf = NULL;
      dcd_event_xfer_complete(rhport,
                              tu_edpt_addr(0, TUSB_DIR_OUT),
                              _dcd.pipe0.length - _dcd.pipe0.remaining,
                              XFER_RESULT_SUCCESS, true);
    }
    return;
  }

  /* 
   * When CSR0 is zero, it means that completion of sending any length packet
   * or receiving a zero length packet
   */
  if (req != REQUEST_TYPE_INVALID && !tu_edpt_dir(req)) {
    /* STATUS IN */
    if (*(const uint16_t*)(uintptr_t)&_dcd.setup_packet == 0x0500) {
      /* The address must be changed on completion of the control transfer. */
      musb_regs->faddr = (uint8_t)_dcd.setup_packet.wValue;
    }
    _dcd.setup_packet.bmRequestType = REQUEST_TYPE_INVALID;
    dcd_event_xfer_complete(rhport,
                            tu_edpt_addr(0, TUSB_DIR_IN),
                            _dcd.pipe0.length - _dcd.pipe0.remaining,
                            XFER_RESULT_SUCCESS, true);
    return;
  }
  if (_dcd.pipe0.buf) {
    /* DATA IN */
    _dcd.pipe0.buf = NULL;
    dcd_event_xfer_complete(rhport,
                            tu_edpt_addr(0, TUSB_DIR_IN),
                            _dcd.pipe0.length - _dcd.pipe0.remaining,
                            XFER_RESULT_SUCCESS, true);
  }
}

/*
 * @brief      Process endpoint x states
 *
 * @param[in] rhport:  Roothub instance being utilized
 *            ep_addr: Address of endpoint utilized
 * 
 * @retval  none
 */
static void process_edpt_n(uint8_t rhport, uint_fast8_t ep_addr)
{
  /*  */
  bool completed;
  const unsigned dir_in     = tu_edpt_dir(ep_addr);
  const unsigned ep_num     = tu_edpt_number(ep_addr);
  const unsigned epn_minus1 = ep_num - 1;

  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, ep_num);
  if (dir_in) {
    // TU_LOG1(" TX CSRL%d = %x\r\n", ep_num, ep_csr->tx_csr1);
    /* If endpoint is stalled then clear the bit and underrun  */
    if (ep_csr->tx_csr1 & MUSB_TXCSRL1_STALLED) {
      ep_csr->tx_csr1 &= ~(MUSB_TXCSRL1_STALLED | MUSB_TXCSRL1_UNDRN);
      return;
    }
    completed = handle_xfer_in(rhport, ep_addr);
  } else {
    // TU_LOG1(" RX CSRL%d = %x\r\n", ep_num, ep_csr->rx_csr1);
    if (ep_csr->rx_csr1 & MUSB_RXCSRL1_STALLED) {
      ep_csr->rx_csr1 &= ~(MUSB_RXCSRL1_STALLED | MUSB_RXCSRL1_OVER);
      return;
    }
    completed = handle_xfer_out(rhport, ep_addr);
  }

  if (completed) {
    pipe_state_t *pipe = &_dcd.pipe[dir_in][epn_minus1];
    dcd_event_xfer_complete(rhport, ep_addr,
                            pipe->length - pipe->remaining,
                            XFER_RESULT_SUCCESS, true);
  }
}

/*
 * @brief      Upon BUS RESET is detected, hardware havs already done:
 *             faddr = 0
 *             index = 0
 *             flushes all ep fifos
 *             clears all ep csr
 *             enabled all ep interrupts
 * 
 * @param[in] rhport:  Roothub instance being utilized
 * 
 * @retval  none
 */
static void process_bus_reset(uint8_t rhport) {
  musb_regs_t* musb = MUSB_REGS(rhport);

  /*
   * When bmRequestType is REQUEST_TYPE_INVALID(0xFF), a control 
   * transfer state is SETUP or STATUS stage.
   */
  _dcd.setup_packet.bmRequestType = REQUEST_TYPE_INVALID;
  _dcd.status_out = 0;
  /* When pipe0.buf has not NULL, DATA stage works in progress. */
  _dcd.pipe0.buf = NULL;

  /* Enable only EP0 */
  musb->intr_txen1 = 1;
  musb->intr_rxen1 = 0;

  /* Clear FIFO settings */
  for (unsigned i = 1; i < TUP_DCD_ENDPOINT_MAX; ++i) {
    musb->index = i;
    // hwfifo_reset(musb, i, 0);
    // hwfifo_reset(musb, i, 1);
  }
  dcd_event_bus_reset(rhport, TUSB_SPEED_FULL, true);
}

/*------------------------------------------------------------------
 * Device API
 *------------------------------------------------------------------*/

#if CFG_TUSB_DEBUG >= MUSB_DEBUG
void print_musb_info(musb_regs_t* musb_regs) {
  musb_regs->index = 0;
  TU_LOG1("config_data0 = 0x%x\r\n", musb_regs->indexed_csr.config_data0);

#if MUSB_CFG_DYNAMIC_FIFO
  TU_LOG1("Dynamic FIFO configuration\r\n");
#else
  for (uint8_t i=1; i <= musb_regs->epinfo_bit.tx_ep_num; i++) {
    musb_regs->index = i;
    TU_LOG1("FIFO %u Size: TX %u RX %u\r\n", i, musb_regs->indexed_csr.fifo_size_bit.tx, musb_regs->indexed_csr.fifo_size_bit.rx);
  }
#endif
}
#endif

/*
 * @brief      Initializes USB IP before beginning any
 *             operation
 *
 * @param[in] rhport:  Roothub instance being utilized
 *            rh_init: Device role and class configuration
 * 
 * @retval  void
 */
bool dcd_init(uint8_t rhport, const tusb_rhport_init_t* rh_init) {
  (void) rh_init;
  musb_regs_t* musb_regs = MUSB_REGS(rhport);

#if CFG_TUSB_DEBUG >= MUSB_DEBUG
  print_musb_info(musb_regs);
#endif

  /* Enable Suspend interrupt */
  musb_regs->intr_usben |= MUSB_IE_SUSPND;
  //TODO: Investigate behavior when enabling SOF/RESET/RESUME interrupt
  /* Clear pending interrupts */
  musb_dcd_int_clear(rhport);
  /* Initialize PHY */
  musb_dcd_phy_init(rhport);
  /* Connect USB device via DP/DM */
  dcd_connect(rhport);

  return true;
}

/*
 * @brief      Enable generic USB interrupts
 *
 * @param[in] rhport:  Roothub instance being utilized
 * 
 * @retval  void
 */
void dcd_int_enable(uint8_t rhport) {
  /* Do testing to confirm if required or not */
  #if(CFG_TUSB_MCU == OPT_MCU_MSPM0C511X)
  MSPM0_ClearPendingIRQ(USB0_IRQn);
  #endif
  musb_dcd_int_enable(rhport);
}

/*
 * @brief      Disable generic USB interrupts
 *
 * @param[in] rhport:  Roothub instance being utilized
 * 
 * @retval  void
 */
void dcd_int_disable(uint8_t rhport) {
  musb_dcd_int_disable(rhport);
}

/*
 * @brief      Receive and process Set Address request
 *
 * @param[in] rhport:   Roothub instance being utilized
 *            dev_addr: Address received from Set Address req
 * 
 * @retval  void
 */
void dcd_set_address(uint8_t rhport, uint8_t dev_addr)
{
  /* dev_addr not utilized, set later within ep0_process() */
  (void)dev_addr;
  musb_regs_t* musb_regs  = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr   = get_ep_csr(musb_regs, 0);

  /* Initializing EP0 status */
  _dcd.pipe0.buf       = NULL;
  _dcd.pipe0.length    = 0;
  _dcd.pipe0.remaining = 0;

  /* TODO: Sanity check the FADDR not being set is ok */
  // musb_regs->faddr = dev_addr;

  /* Clear RX FIFO to return ACK. */
  ep_csr->csr0 = MUSB_CSR0_RXPKTRDYC | MUSB_CSR0_DATAEND;
}

/*
 * @brief      Check if device is in SUSPEND mode and if
 *             it is, then enable RESUME bit for approx
 *             10ms.
 *
 * @param[in] rhport  Roothub instance being utilized
 * 
 * @retval  void
 */
void dcd_remote_wakeup(uint8_t rhport)
{
  (void) rhport;
  // musb_regs_t* musb_regs = MUSB_REGS(rhport);

  // /* If USB is not in Suspend mode do nothing and return */
  // if (!(musb_regs->power & MUSB_POWER_SUSPEND))
  //   return;

  // /* Grab the current time */
  // uint32_t cnt = board_millis();

  // /* 
  //  * If USB is in Suspend mode and a remote wakeup
  //  * is requested. Resume bit must be set to 1 for
  //  * approx. 10ms. (min: 2ms | max: 15ms)
  //  * Once done reset it to 0.
  //  */
  // musb_regs->power |= MUSB_POWER_RESUME;

  // /* 
  //  * If difference between new time and original time is
  //  * not 10 or more then do nothing.
  //  */
  // while (board_millis() - cnt < 10)
  //   ;

  // /* Once done, set the RESUME bit to 0 */
  // musb_regs->power &= ~MUSB_POWER_RESUME;
}

/*
 * @brief      Connect USB device through enabling pull-up
 *             resistors on D+/D-
 *
 * @param[in] rhport  Roothub instance being utilized
 * 
 * @retval  void
 */
void dcd_connect(uint8_t rhport)
{
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_regs->power |= 0;

  /* Specific to only MSPM0C511X*/
#ifdef OPT_MCU_MSPM0C511X
  USBLC0->USBMODE |= MUSB_POWER_SOFTCONN;
#endif
}

/*
 * @brief      Disconnect USB device through disabling pull-up
 *             resistors on D+/D-
 *
 * @param[in] rhport  Roothub instance being utilized
 * 
 * @retval  void
 */
void dcd_disconnect(uint8_t rhport)
{
  (void) rhport;

  /* Specific to only MSPM0C511X*/
#ifdef OPT_MCU_MSPM0C511X
  USBLC0->USBMODE &= ~MUSB_POWER_SOFTCONN;
#endif
}

void dcd_sof_enable(uint8_t rhport, bool en)
{
  (void) rhport;
  (void) en;

  // TODO implement later
}

//--------------------------------------------------------------------+
// Endpoint API
//--------------------------------------------------------------------+
// static void edpt_setup(musb_regs_t* musb, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_size){
//   const unsigned epn     = tu_edpt_number(ep_addr);
//   const unsigned dir_in  = tu_edpt_dir(ep_addr);
// }

/*
 * @brief      Configure endpoint's registers according
 *             to descriptor
 *
 * @param[in] rhport:  Roothub instance being utilized
 *            ep_desc: pointer to endpoint descriptor
 * 
 * @retval  true:   success
 *          false:  failed
 */
bool dcd_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const * ep_desc) {
  /* Grab address of current endpoint */
  const unsigned ep_addr          = ep_desc->bEndpointAddress;
  /* Determine endpoint number based off endpoint address */
  const unsigned ep_num           = tu_edpt_number(ep_addr);
  /* Direction of current endpoint */
  const unsigned dir_in           = tu_edpt_dir(ep_addr);
  /* Check direction of current endpoint */
  const unsigned is_rx            = (1 - dir_in);
  /* Check max packet size of current endpoint */
  const unsigned max_packet_size  = tu_edpt_packet_size(ep_desc);

  /* Initializing data structure for current endpoint */
  pipe_state_t *pipe = &_dcd.pipe[dir_in][ep_num - 1];
  pipe->buf       = NULL;
  pipe->length    = 0;
  pipe->remaining = 0;

  musb_regs_t* musb = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb, ep_num);

  /* EP_DIR is 1 then the endpoint is a RX endpoint  */
  if (is_rx) {
    ep_csr->rx_max_ep = max_packet_size;
    ep_csr->rx_csr2 = 0;
  } else {
    ep_csr->tx_max_ep = max_packet_size;
    ep_csr->tx_csr2 = 0;
  }

  /* Flush respective endpoint FIFO */
  hwfifo_flush(musb, ep_num, is_rx, true);

  //Most likely not needed with this version of the IP
  // TU_ASSERT(hwfifo_config(musb, ep_num, is_rx, max_packet_size, false));

  /* Enable interrupt for endpoint */
  if (is_rx){
    musb->intr_rxen1 |= TU_BIT(ep_num);
  } else {
    musb->intr_txen1 |= TU_BIT(ep_num);
  }
    
  return true;
}

//todo
bool dcd_edpt_iso_alloc(uint8_t rhport, uint8_t ep_addr, uint16_t largest_packet_size) {
  const unsigned epn    = tu_edpt_number(ep_addr);
  const unsigned dir_in = tu_edpt_dir(ep_addr);
  musb_regs_t* musb = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb, epn);
  const uint8_t is_rx = 1 - dir_in;
  if (is_rx) {
    ep_csr->rx_csr2 = 0;
  } else {
    ep_csr->tx_csr2 = 0;
  }
  return hwfifo_config(musb, epn, is_rx, largest_packet_size, true);
}

//todo
bool dcd_edpt_iso_activate(uint8_t rhport, tusb_desc_endpoint_t const *ep_desc ) {
  const unsigned ep_addr = ep_desc->bEndpointAddress;
  const unsigned epn     = tu_edpt_number(ep_addr);
  const unsigned dir_in  = tu_edpt_dir(ep_addr);
  const unsigned mps     = tu_edpt_packet_size(ep_desc);

  unsigned const ie = musb_dcd_get_int_enable(rhport);
  musb_dcd_int_disable(rhport);

  pipe_state_t *pipe = &_dcd.pipe[dir_in][epn - 1];
  pipe->buf       = NULL;
  pipe->length    = 0;
  pipe->remaining = 0;

  musb_regs_t* musb = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb, epn);
  const uint8_t is_rx = 1 - dir_in;

  /*  */
  if (is_rx) {
    ep_csr->rx_max_ep = mps;
    ep_csr->rx_csr2 |= MUSB_CSRH_ISO;
    hwfifo_flush(musb, epn, is_rx, true);
    musb->intr_rxen1 |= TU_BIT(epn);
  } else {
    ep_csr->tx_max_ep = mps;
    ep_csr->tx_csr2 |= MUSB_CSRH_ISO;
    hwfifo_flush(musb, epn, is_rx, true);
    musb->intr_txen1 |= TU_BIT(epn);
  }

  if (ie) musb_dcd_int_enable(rhport);

  return true;
}

/*
 * @brief      Close an endpoint
 *
 * @param[in] rhport:  Roothub instance being utilized
 *            ep_addr: Address of endpoint
 * 
 * @retval  None
 */
// void dcd_edpt_close(uint8_t rhport, uint8_t ep_addr)
// {
//   musb_regs_t* musb       = MUSB_REGS(rhport);
//   const unsigned ep_num   = tu_edpt_number(ep_addr);
//   const unsigned is_rx    = (1 - tu_edpt_dir(ep_addr));

//   /* Before disabling common USB interrupts, grab current status */
//   unsigned const ie = musb_dcd_get_int_enable(rhport);
//   /* Disable common USB interrupts */
//   musb_dcd_int_disable(rhport);

//   /* Disable the selected endpoint */
//   if (is_rx) {
//     musb->intr_rxen1 &= ~(1 << ep_num);
//   }
// }

/*
 * @brief      Close all non-control endpoints, cancel
 *             all pending transfers if any.
 *
 * @param[in] rhport:  Roothub instance being utilized
 * 
 * @retval  None
 */
void dcd_edpt_close_all(uint8_t rhport)
{
  musb_regs_t* musb = MUSB_REGS(rhport);

  /* Before disabling common USB interrupts, grab current status */
  unsigned const ie = musb_dcd_get_int_enable(rhport);
  /* Disable common USB interrupts */
  musb_dcd_int_disable(rhport);
  /* Keep control transfer endpoint (EP0) active tx active */
  musb->intr_txen1 = 1;
  /* Bit 0 or EP0 is considered reserved, so set this to 0 */
  musb->intr_rxen1 = 0;

  /* 
   * Iterate through all OUT/IN endpoints to disable and
   * flush their fifo
   */
  for (unsigned i = 1; i < TUP_DCD_ENDPOINT_MAX; ++i) {
    /* Grab current endpoint control/status register */
    musb_ep_csr_t* ep_csr = get_ep_csr(musb, i);

    /* Flush IN endpoint */
    hwfifo_flush(musb, i, 0, true);
    /* Clear maximum packet size and control/status register */
    ep_csr->tx_max_ep = 0;
    ep_csr->tx_csr1 = 0;

    /* Flush OUT endpoint */
    hwfifo_flush(musb, i, 1, true);
    /* Clear maximum packet size and control/status register */
    ep_csr->rx_max_ep = 0;
    ep_csr->rx_csr1 = 0;

  }

  /* Re-enable interrupt if was previously enabled */
  if (ie) musb_dcd_int_enable(rhport);
}

/*
 * @brief     Submit a transfer, When complete dcd_event_xfer_complete()
 *            is invoked to notify the stack
 *
 * @param[in] rhport:  Roothub instance being utilized
 * 
 * @retval  None
 */
bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t * buffer, uint16_t total_bytes)
{
  (void)rhport;
  bool ret;
  // TU_LOG1("X %x %d\r\n", ep_addr, total_bytes);
  /* Grab the endpoint we want to do transfer */
  unsigned const ep_num = tu_edpt_number(ep_addr);
  /* Check if common USB interrupts are enabled */
  unsigned const ie     = musb_dcd_get_int_enable(rhport);
  /* Disable common interrupt */
  musb_dcd_int_disable(rhport);

  /* If EP 0, then EP 0 specific API will be called otherwise generic API is called */
  if (ep_num) {
    _dcd.pipe_buf_is_fifo[tu_edpt_dir(ep_addr)] &= ~TU_BIT(ep_num - 1);
    ret = edpt_n_xfer(rhport, ep_addr, buffer, total_bytes);
  } else {
    ret = edpt0_xfer(rhport, ep_addr, buffer, total_bytes);
  }

  /* Re-enable interrupt if previously enabled */
  if (ie) musb_dcd_int_enable(rhport);

  return ret;
}

/*
 * @brief     Submit a transfer where is managed by FIFO, When complete
 *            dcd_event_xfer_complete() is invoked to notify the stack
 *
 * @note      Optional, however, must be listed in usbd.c
 *            is invoked to notify the stack
 *
 * @param[in] rhport:  Roothub instance being utilized
 * 
 * @retval  None
 */
bool dcd_edpt_xfer_fifo(uint8_t rhport, uint8_t ep_addr, tu_fifo_t * ff, uint16_t total_bytes)
{
  (void)rhport;
  bool ret;
  // TU_LOG1("X %x %d\r\n", ep_addr, total_bytes);
  /* Grab endpoint number based off address */
  unsigned const ep_num = tu_edpt_number(ep_addr);

  /* Ensure it's not endpoint 0 */
  TU_ASSERT(ep_num);

  /* Grab status of common USB interrupts before disabling */
  unsigned const ie = musb_dcd_get_int_enable(rhport);
  musb_dcd_int_disable(rhport);

  /*
   * Update data structure to indicate the IN or OUT endpoint
   * utilizes a FIFO
   */
  _dcd.pipe_buf_is_fifo[tu_edpt_dir(ep_addr)] |= TU_BIT(ep_num - 1);
  ret = edpt_n_xfer(rhport, ep_addr, (uint8_t*)ff, total_bytes);

  /* Re-enable interrupt if they were previously enabled */
  if (ie) musb_dcd_int_enable(rhport);

  return ret;
}

/*
 * @brief     Stalls endpoint x
 *
 * @param[in] rhport:  Roothub instance being utilized
 *            ep_addr: address of endpoint we wish to stall
 * 
 * @retval  None
 */
void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr) {
  unsigned const ie = musb_dcd_get_int_enable(rhport);
  musb_dcd_int_disable(rhport);
  unsigned const epn = tu_edpt_number(ep_addr);
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, epn);

  const uint8_t is_rx = 1 - tu_edpt_dir(ep_addr);
  if (is_rx) {
    ep_csr->rx_csr1 = MUSB_CSR1_SEND_STALL(is_rx);
  } else {
    ep_csr->tx_csr1 = MUSB_CSR1_SEND_STALL(is_rx);
  }

  if (ie) musb_dcd_int_enable(rhport);
}

// clear stall, data toggle is also reset to DATA0
void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr)
{
  (void)rhport;
  unsigned const ie = musb_dcd_get_int_enable(rhport);
  musb_dcd_int_disable(rhport);
  unsigned const epn = tu_edpt_number(ep_addr);
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  musb_ep_csr_t* ep_csr = get_ep_csr(musb_regs, epn);
  const uint8_t is_rx = 1 - tu_edpt_dir(ep_addr);

  if (is_rx) {
    ep_csr->rx_csr1 = MUSB_CSR1_CLEAR_DATA_TOGGLE(is_rx);
  } else {
    ep_csr->tx_csr1 = MUSB_CSR1_CLEAR_DATA_TOGGLE(is_rx);
  }

  if (ie) musb_dcd_int_enable(rhport);

}

/*-------------------------------------------------------------------
 * ISR
 *-------------------------------------------------------------------*/
void dcd_int_handler(uint8_t rhport) {
  musb_regs_t* musb_regs = MUSB_REGS(rhport);
  /* Save endpoint index for later */
  const uint8_t saved_index = musb_regs->index;

  /* Part specific ISR setup/entry */
  musb_dcd_int_handler_enter(rhport);

  /* Read common USB interrupts, upon read interrupt status will be cleared */
  uint_fast8_t intr_usb = musb_regs->intr_usb;
#if (TUP_DCD_ENDPOINT_MAX <= 8)
  /* Read interrupt for IN endpoints, upon read interrupt status will be cleared */
  uint_fast8_t intr_tx = get_intr_tx_status(musb_regs);
  /* Read interrupt for OUT endpoints, upon read interrupt status will be cleared */
  uint_fast8_t intr_rx = get_intr_rx_status(musb_regs);
#else
  /* Read interrupt for IN endpoints, upon read interrupt status will be cleared */
  uint_fast16_t intr_tx = get_intr_tx_status(musb_regs);
  /* Read interrupt for OUT endpoints, upon read interrupt status will be cleared */
  uint_fast16_t intr_rx = get_intr_rx_status(musb_regs);
#endif

  /* Clear disabled IntrUSBE interrupts */
  intr_usb &= musb_regs->intr_usben;

  /* Process common USB interrupts first */
  if (intr_usb & MUSB_IS_RESUME) {
    dcd_event_bus_signal(rhport, DCD_EVENT_RESUME, true);
  }
  if (intr_usb & MUSB_IS_RESET) {
    process_bus_reset(rhport);
  }
  if (intr_usb & MUSB_IS_SOF) {
    dcd_event_bus_signal(rhport, DCD_EVENT_SOF, true);
  }

  /* Clear disabled tx interrupts */
  intr_tx &= ((musb_regs->intr_txen2 << 8) | musb_regs->intr_txen1);

  /* Process endpoint 0 first */
  if (intr_tx & TU_BIT(0)) {
    process_ep0(rhport);
    intr_tx &= ~TU_BIT(0);
  }

  /* Process any other endpoint besides 0 */
  while (intr_tx) {
    unsigned const num = __builtin_ctz(intr_tx);
    process_edpt_n(rhport, tu_edpt_addr(num, TUSB_DIR_IN));
    intr_tx &= ~TU_BIT(num);
  }

  /* Clear disabled rx interrupts */
  intr_rx &= ((musb_regs->intr_rxen2 << 8) | musb_regs->intr_rxen1);

  while (intr_rx) {
    unsigned const num = __builtin_ctz(intr_rx);
    process_edpt_n(rhport, tu_edpt_addr(num, TUSB_DIR_OUT));
    intr_rx &= ~TU_BIT(num);
  }

  /* SUSPEND interrupt should be serviced last  */
  if (intr_usb & MUSB_IS_SUSPEND) {
    dcd_event_bus_signal(rhport, DCD_EVENT_SUSPEND, true);
  }

  musb_regs->index = saved_index; // restore endpoint index
}
