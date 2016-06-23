
#ifndef I2C_TWI_H_
#define I2C_TWI_H_

#include <assert.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <util/twi.h>

#define I2C_FREQ 100000

#define I2C_TXN_DONE _BV(0)
#define I2C_TXN_ERR  _BV(1)

typedef struct i2c_op i2c_op_t;
typedef struct i2c_txn i2c_txn_t;

struct i2c_op {
	uint8_t address;
	uint8_t buflen;
	uint8_t bufpos;
	uint8_t *buf;
};

struct i2c_txn {
	struct i2c_txn *next;
	volatile uint8_t flags;
	uint8_t opslen;
	uint8_t opspos;
	struct i2c_op ops[];
};

static inline void i2c_op_init(i2c_op_t *o, uint8_t address, uint8_t *buf, uint8_t buflen) {
	o->address = address;
	o->buflen = buflen;
	o->bufpos = 0;
	o->buf = buf;
}

static inline void i2c_op_init_rd(i2c_op_t *o, uint8_t address, uint8_t *buf, uint8_t buflen) {
	i2c_op_init(o, (address << 1) | TW_READ, buf, buflen);
}

static inline void i2c_op_init_wr(i2c_op_t *o, uint8_t address, uint8_t *buf, uint8_t buflen) {
	i2c_op_init(o, (address << 1) | TW_WRITE, buf, buflen);
}

static inline void i2c_txn_init(i2c_txn_t *t, uint8_t opslen) {
	t->flags = 0;
	t->opslen = opslen;
	t->opspos = 0;
	t->next = NULL;
}

void i2c_init(void);
void i2c_post(i2c_txn_t *t);

/* Pointers to current txn and op. */
static volatile i2c_txn_t *txn;
static volatile i2c_op_t *op;

/*
 * By default, the control register is set to:
 *  - TWEA: Automatically send acknowledge bit in receive mode.
 *  - TWEN: Enable the I2C system.
 *  - TWIE: Enable interrupt requests when TWINT is set.
 */
#define TWCR_DEFAULT (_BV(TWEA) | _BV(TWEN) | _BV(TWIE))

#define TWCR_NOT_ACK (_BV(TWINT) | _BV(TWEN) | _BV(TWIE))
#define TWCR_ACK (TWCR_NOT_ACK | _BV(TWEA))

void i2c_init(void) {
  uint8_t sreg;

  /* Store the status register and disable interrupts. */
  sreg = SREG;
  cli();

  /*
   * From ATmega328P datasheet:
   *   SCL freq = (CPU Clock freq) / (16 + 2(TWBR) * (PrescalerValue))
   *
   * Which means:
   *   TWBR = ((CPU Clock freq) / (SCL freq) - 16) / (2 * (PrescalerValue))
   *
   * Disable the prescaler and set TWBR according to CPU freq and SCL freq.
   */
  TWSR &= ~(_BV(TWPS1) | _BV(TWPS0));
  TWBR = ((F_CPU / I2C_FREQ) - 16) / (2 * 1);

  /*
   * Active internal pull-up resistors for SCL and SDA.
   * Their ports are PC5 for SCL and PC4 for SDA on the ATmega328P.
   */
  PORTC |= _BV(PC5) | _BV(PC4);

  /* Enable interrupts via the control register. */
  TWCR = TWCR_DEFAULT;

  /* Disable slave mode. */
  TWAR = 0;

  /* Restore the status register. */
  SREG = sreg;
}

void i2c_post(i2c_txn_t *t) {
  uint8_t sreg;

  /* Reset transaction attributes. */
  t->flags = 0;
  t->opspos = 0;
  t->next = NULL;

  sreg = SREG;
  cli();

  /* Append transaction to linked list. */
  if (txn == NULL) {
    txn = t;
    op = &txn->ops[0];

    /* Transmit START to kickstart things. */
    TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
  } else {
    volatile i2c_txn_t *txn_ = txn;
    while (txn_ != NULL) {
      if (txn_->next != NULL) {
        txn_ = txn_->next;
      } else {
        txn_->next = t;
        break;
      }
    }
  }

  SREG = sreg;
}

ISR(TWI_vect, ISR_BLOCK) {
  uint8_t status = TW_STATUS;

  /* This interrupt should only fire if there is something to do. */
  assert(op != NULL);

  if ((op->address & _BV(0)) == TW_READ) {
    /* Master Receiver mode. */
    switch (status) {

    /* A START condition has been transmitted. */
    case TW_START:
    /* A repeated START condition has been transmitted. */
    case TW_REP_START:
      assert(op->buflen > 0);
      op->bufpos = 0;
      TWDR = op->address;
      TWCR = TWCR_DEFAULT | _BV(TWINT);
      break;

    /* Arbitration lost in SLA+R or NOT ACK bit. */
    case TW_MR_ARB_LOST:
      /* A START condition will be transmitted when the bus becomes free. */
      TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
      break;

    /* SLA+R has been transmitted; ACK has been received. */
    case TW_MR_SLA_ACK:
      if (op->buflen == 1) {
        TWCR = TWCR_NOT_ACK;
      } else {
        TWCR = TWCR_ACK;
      }
      break;

    /* SLA+R has been transmitted; NOT ACK has been received. */
    case TW_MR_SLA_NACK:
      txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
      goto next_txn;

    /* Data byte has been received; ACK has been returned. */
    case TW_MR_DATA_ACK:
      op->buf[op->bufpos++] = TWDR;
      if (op->bufpos+1 == op->buflen) {
        TWCR = TWCR_NOT_ACK;
      } else {
        TWCR = TWCR_ACK;
      }
      break;

    /* Data byte has been received; NOT ACK has been returned. */
    case TW_MR_DATA_NACK:
      op->buf[op->bufpos++] = TWDR;
      goto next_op;

    default:
      assert(0 && "unknown status in master receiver mode");
    }
  } else {
    /* Master Transmitter mode. */
    switch (status) {

    /* A START condition has been transmitted. */
    case TW_START:
    /* A repeated START condition has been transmitted. */
    case TW_REP_START:
      assert(op->buflen > 0);
      op->bufpos = 0;
      TWDR = op->address;
      TWCR = TWCR_DEFAULT | _BV(TWINT);
      break;

    /* Arbitration lost in SLA+W or data bytes. */
    case TW_MT_ARB_LOST:
      /* A START condition will be transmitted when the bus becomes free. */
      TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
      break;

    /* SLA+W has been transmitted; ACK has been received. */
    case TW_MT_SLA_ACK:
      TWDR = op->buf[op->bufpos++];
      TWCR = TWCR_DEFAULT | _BV(TWINT);
      break;

    /* SLA+W has been transmitted; NOT ACK has been received. */
    case TW_MT_SLA_NACK:
      txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
      goto next_txn;

    /* Data byte has been transmitted; ACK has been received. */
    case TW_MT_DATA_ACK:
      if (op->bufpos < op->buflen) {
        TWDR = op->buf[op->bufpos++];
        TWCR = TWCR_DEFAULT | _BV(TWINT);
        break;
      }

      /* No more bytes left to transmit... */
      goto next_op;

    /* Data byte has been transmitted; NOT ACK has been received. */
    case TW_MT_DATA_NACK:
      if (op->bufpos < op->buflen) {
        /* There were more bytes left to transmit! */
        txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
        goto next_txn;
      }

      goto next_op;

    default:
      assert(0 && "unknown status in master transmitter mode");
    }
  }

  return;

next_op:
  /*
   * Advance to next operation in transaction, if possible.
   */
  if (++(txn->opspos) < txn->opslen) {
    op = &txn->ops[txn->opspos];

    /* Repeated start. */
    TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
    return;
  }

  /* No more operations, mark transaction as done. */
  txn->flags = I2C_TXN_DONE;

next_txn:
  /*
   * Advance to next transaction, if possible.
   */
  if (txn->next != NULL) {
    txn = txn->next;
    op = &txn->ops[0];

    /* Repeated start. */
    TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
    return;
  }

  txn = NULL;
  op = NULL;

  /* No more transaction, transmit STOP. */
  TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTO);
}


#endif /* I2C_TWI_H_ */