#ifndef PACKET_ANNO_H
#define PACKET_ANNO_H
/*
 * Click packet annotations.
 */


#define EXTRA_HEADER_CB_OFFSET 16
#define CB_SIZE 48
#define WIFI_EXTRA_MAGIC  0x7492001
enum {
  WIFI_EXTRA_TX                    = (1<<0), // 1
  WIFI_EXTRA_TX_FAIL               = (1<<1), // 2
  WIFI_EXTRA_TX_USED_ALT_RATE      = (1<<2), // 4
  WIFI_EXTRA_RX_ERR                = (1<<3), // 8
  WIFI_EXTRA_RX_MORE               = (1<<4), // 16
  WIFI_EXTRA_NO_SEQ                = (1<<5),
  WIFI_EXTRA_NO_TXF                = (1<<6),
  WIFI_EXTRA_DO_RTS_CTS            = (1<<7),
  WIFI_EXTRA_DO_CTS                = (1<<8),
};



struct click_wifi_extra {
  u_int32_t magic;
  u_int32_t flags;

  u_int8_t rssi;
  u_int8_t silence;
  u_int8_t power;
  u_int8_t pad;

  u_int8_t rate;
  u_int8_t rate1;
  u_int8_t rate2;
  u_int8_t rate3;

  u_int8_t max_retries;
  u_int8_t max_retries1;
  u_int8_t max_retries2;
  u_int8_t max_retries3;

  u_int8_t virt_col;
  u_int8_t retries;
  u_int16_t len;
};


typedef struct {
  u_int32_t did;
  u_int16_t status;
  u_int16_t len;
  u_int32_t data;
} p80211item_uint32_t;

typedef struct {
  u_int32_t msgcode;
  u_int32_t msglen;
#define WLAN_DEVNAMELEN_MAX 16
  u_int8_t devname[WLAN_DEVNAMELEN_MAX];
  p80211item_uint32_t hosttime;
  p80211item_uint32_t mactime;
  p80211item_uint32_t channel;
  p80211item_uint32_t rssi;
  p80211item_uint32_t sq;
  p80211item_uint32_t signal;
  p80211item_uint32_t noise;
  p80211item_uint32_t rate;
  p80211item_uint32_t istx;
  p80211item_uint32_t frmlen;
} wlan_ng_prism2_header;


struct click_wifi {
	u_int8_t	i_fc[2];
	u_int8_t	i_dur[2];
	u_int8_t	i_addr1[6];
	u_int8_t	i_addr2[6];
	u_int8_t	i_addr3[6];
	u_int8_t	i_seq[2];
};

#define	WIFI_FC0_VERSION_MASK		0x03
#define	WIFI_FC0_VERSION_0		0x00
#define	WIFI_FC0_TYPE_MASK		0x0c
#define	WIFI_FC0_TYPE_MGT		0x00
#define	WIFI_FC0_TYPE_CTL		0x04
#define	WIFI_FC0_TYPE_DATA		0x08

#define	WIFI_FC0_SUBTYPE_MASK		0xf0
/* for TYPE_MGT */
#define	WIFI_FC0_SUBTYPE_ASSOC_REQ	0x00
#define	WIFI_FC0_SUBTYPE_ASSOC_RESP	0x10
#define	WIFI_FC0_SUBTYPE_REASSOC_REQ	0x20
#define	WIFI_FC0_SUBTYPE_REASSOC_RESP	0x30
#define	WIFI_FC0_SUBTYPE_PROBE_REQ	0x40
#define	WIFI_FC0_SUBTYPE_PROBE_RESP	0x50
#define	WIFI_FC0_SUBTYPE_BEACON		0x80
#define	WIFI_FC0_SUBTYPE_ATIM		0x90
#define	WIFI_FC0_SUBTYPE_DISASSOC	0xa0
#define	WIFI_FC0_SUBTYPE_AUTH		0xb0
#define	WIFI_FC0_SUBTYPE_DEAUTH		0xc0
/* for TYPE_CTL */
#define	WIFI_FC0_SUBTYPE_PS_POLL	0xa0
#define	WIFI_FC0_SUBTYPE_RTS		0xb0
#define	WIFI_FC0_SUBTYPE_CTS		0xc0
#define	WIFI_FC0_SUBTYPE_ACK		0xd0
#define	WIFI_FC0_SUBTYPE_CF_END		0xe0
#define	WIFI_FC0_SUBTYPE_CF_END_ACK	0xf0
/* for TYPE_DATA (bit combination) */
#define WIFI_FC0_SUBTYPE_DATA		0x00
#define	WIFI_FC0_SUBTYPE_CF_ACK		0x10
#define	WIFI_FC0_SUBTYPE_CF_POLL	0x20
#define	WIFI_FC0_SUBTYPE_CF_ACPL	0x30
#define	WIFI_FC0_SUBTYPE_NODATA		0x40
#define	WIFI_FC0_SUBTYPE_CFACK		0x50
#define	WIFI_FC0_SUBTYPE_CFPOLL		0x60
#define	WIFI_FC0_SUBTYPE_CF_ACK_CF_ACK	0x70

#define	WIFI_FC1_DIR_MASK		0x03
#define	WIFI_FC1_DIR_NODS		0x00	/* STA->STA */
#define	WIFI_FC1_DIR_TODS		0x01	/* STA->AP  */
#define	WIFI_FC1_DIR_FROMDS		0x02	/* AP ->STA */
#define	WIFI_FC1_DIR_DSTODS		0x03	/* AP ->AP  */

#define	WIFI_FC1_MORE_FRAG		0x04
#define	WIFI_FC1_RETRY			0x08
#define	WIFI_FC1_PWR_MGT		0x10
#define	WIFI_FC1_MORE_DATA		0x20
#define	WIFI_FC1_WEP			0x40
#define	WIFI_FC1_ORDER			0x80

#define	WIFI_NWID_LEN			32

#endif /* PACKET_ANNO_H */
