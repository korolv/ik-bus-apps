/* 
 * File:   ikbusframe.h
 * Author: Vladimir Korol
 *
 * Created on March 4, 2015
 * 
 * Most of these data were obtained from navcoder. www.navcoder.com
 */

#ifndef _IK_BUS_FRAME_H_
#define	_IK_BUS_FRAME_H_

#define IKBUS_FRM_SENDER         0
#define IKBUS_FRM_SIZE           1
#define IKBUS_FRM_RECEIVER       2
#define IKBUS_FRM_CMD            3
#define IKBUS_FRM_DATA           4

/* I/K bus devices */
#define IKBUS_DEV_GM             0x00	/*Body module */
#define IKBUS_DEV_MID1           0x01	/* Multi-Info Display (1st generation) */
#define IKBUS_DEV_EKM            0x02	/* Body Electronics Module */
#define IKBUS_DEV_SHD            0x08	/* Sunroof */
#define IKBUS_DEV_ZKE            0x09	/* Central Body Electronics */
#define IKBUS_DEV_DME            0x12	/* Digital motor electronics (ECU) */
#define IKBUS_DEV_CDC            0x18	/* CD changer */
#define IKBUS_DEV_HKM            0x24	/* Boot lid control unit */
#define IKBUS_DEV_FUM            0x28	/* Radio clock control module  */
#define IKBUS_DEV_CCM            0x30	/* Check control module */
#define IKBUS_DEV_EGS            0x32	/* Electronic transmission control */
#define IKBUS_DEV_GT             0x3b	/* Graphics driver */
#define IKBUS_DEV_DIA            0x3f	/* Diagnostics */
#define IKBUS_DEV_FBZV           0x40	/* Radio-control system (Remote central locking) */
#define IKBUS_DEV_GTF            0x43	/* Rear graphics driver */
#define IKBUS_DEV_EWS            0x44	/* EWSIII immobiliser control unit */
#define IKBUS_DEV_CID            0x46	/* Central information display */
#define IKBUS_DEV_FMBT           0x47	/* Rear monitor controls */
#define IKBUS_DEV_MFL            0x50	/* Multi-function steering wheel */
#define IKBUS_DEV_MML            0x51	/* Mirror memory Left */
#define IKBUS_DEV_ASC            0x55	/* Anti-lock braking system with ASC */
#define IKBUS_DEV_LWS            0x57	/* Steering angle sensor */
#define IKBUS_DEV_IHKA           0x5b	/* Heater + automatic A/C control */
#define IKBUS_DEV_PDC            0x60	/* Park distance control */
#define IKBUS_DEV_EKP            0x65	/* Electronically controlled fuel pump control unit */
#define IKBUS_DEV_AHL            0x66	/* Adaptive headlight control unit */
#define IKBUS_DEV_ONL            0x67	/*  */
#define IKBUS_DEV_RAD            0x68	/* Radio */
#define IKBUS_DEV_DSP            0x6a	/* Digital sound processor amplifier */
#define IKBUS_DEV_STH            0x6b	/* Park heating */
#define IKBUS_DEV_RDC            0x70	/* Tyre pressure monitoring system */
#define IKBUS_DEV_SM             0x72	/* Seat memory */
#define IKBUS_DEV_SDRS           0x73	/* Satelite digital radio */
#define IKBUS_DEV_SOR            0x74	/* Seat occupancy recognition unit */
#define IKBUS_DEV_CDCD           0x77	/* CD changer DIN */
#define IKBUS_DEV_NAV            0x7f	/* Navigation (Europe) */
#define IKBUS_DEV_IKE            0x80	/* Integrated instrument cluster electronics */
#define IKBUS_DEV_RCSC           0x81	/* Revolution counter/steering column */
#define IKBUS_DEV_HAC            0x9a	/* Headlight aim control */
#define IKBUS_DEV_MMR            0x9b	/* Mirror memory Right */
#define IKBUS_DEV_CVM            0x9c	/* Cabrio folding top module */
#define IKBUS_DEV_FID            0xa0	/* Multi-information display, rear */
#define IKBUS_DEV_AB             0xa4	/* Airbag control module */
#define IKBUS_DEV_GR             0xa6	/* Cruise control */
#define IKBUS_DEV_FHK            0xa7	/* Rear climate control */
#define IKBUS_DEV_NAC            0xa8	/* Navigation system (Chinese) */
#define IKBUS_DEV_EHC            0xac	/* Electronic height control */
#define IKBUS_DEV_SES            0xb0	/* Voice recognition unit */
#define IKBUS_DEV_NAJ            0xbb	/* Navigation system (Japanese) */
#define IKBUS_DEV_GLO            0xbf	/* Global */
#define IKBUS_DEV_MID            0xc0	/* Multi-information display */
#define IKBUS_DEV_TEL            0xc8	/* Telephone */
#define IKBUS_DEV_TCU            0xca	/* Telematics Control Unit */
#define IKBUS_DEV_LCM            0xd0	/* Light check module */
#define IKBUS_DEV_SMAD           0xdb	/* Seat memory assistant driver */
#define IKBUS_DEV_IRIS           0xe0	/* Integrated radio information system */
#define IKBUS_DEV_ANZV           0xe7	/* Display group */
#define IKBUS_DEV_RLS            0xe8	/* Rain and headlight sensor */
#define IKBUS_DEV_DSPC           0xea	/* Digital signal processor controller */
#define IKBUS_DEV_VID            0xed	/* Video module */
#define IKBUS_DEV_BMBT           0xf0	/* On-board monitor control panel */
#define IKBUS_DEV_PIC            0xf1	/* Programmable Controller (custom device) */
#define IKBUS_DEV_CSU            0xf5	/* Centre switch control unit */
#define IKBUS_DEV_LOC            0xff	/* Local */

/* I/K bus commands */
#define IKBUS_MSG_DEV_STAT_REQ          0x01	/* Device status request */
#define IKBUS_MSG_DEV_STAT_READY        0x02	/* Device status ready */
#define IKBUS_MSG_BUS_STAT_REQ          0x03	/* Bus status request */
#define IKBUS_MSG_BUS_STAT              0x04	/* Bus status */
#define IKBUS_MSG_BACKLIGHT_CTL         0x05	/* Backlight Control */
#define IKBUS_MSG_IDENTIFICATION        0x06	/* Identification */
#define IKBUS_MSG_GONG_STAT             0x07	/* Gong status */
#define IKBUS_MSG_VEHICLE_CTL           0x0c	/* Vehicle control */
#define IKBUS_MSG_IGN_STAT_REQ          0x10	/* Ignition status request */
#define IKBUS_MSG_IGN_STAT              0x11	/* Ignition status */
#define IKBUS_MSG_IKE_SENS_STAT_REQ     0x12	/* IKE sensor status request */
#define IKBUS_MSG_IKE_SENS_STAT         0x13	/* IKE sensor status */
#define IKBUS_MSG_CNTRY_COD_STAT_REQ    0x14	/* Country coding status request */
#define IKBUS_MSG_CHTRY_COD_STAT        0x15	/* Country coding status */
#define IKBUS_MSG_ODOMETER_REQ          0x16	/* Odometer request */
#define IKBUS_MSG_ODOMETER              0x17	/* Odometer */
#define IKBUS_MSG_SPEED_RPM             0x18	/* Speed/RPM */
#define IKBUS_MSG_TEMP                  0x19	/* Temperature */
#define IKBUS_MSG_TEXT_GONG             0x1a	/* IKE Text/Gong */
#define IKBUS_MSG_TEXT_STAT             0x1b	/* IKE text status */
#define IKBUS_MSG_GONG                  0x1c	/* Gong */
#define IKBUS_MSG_TEMP_REQ              0x1d	/* Temperature request */
#define IKBUS_MSG_TIME_DATE             0x1f	/* Time & date */
#define IKBUS_MSG_DYSPLAY_STAT          0x20	/* Display status */
#define IKBUS_MSG_MENU_TEXT             0x21	/* Menu Text */
#define IKBUS_MSG_TEXT_DYSP_CONF        0x22	/* Text display confirmation */
#define IKBUS_MSG_TEXT_DYSP             0x23	/* Display Text */
#define IKBUS_MSG_UPDATE_TEXT           0x24	/* Update Text */
#define IKBUS_MSG_MID_REQ               0x27	/* MID display request */
#define IKBUS_MSG_MID_DEN_ACS           0x28	/* MID denied access */
#define IKBUS_MSG_MID_REP_DYSP          0x29	/* Report MID display */
#define IKBUS_MSG_BC_SPEC_IND           0x2A	/* On-board computer special indicators */
#define IKBUS_MSG_TEL_IND               0x2b	/* Telephone indicators */
#define IKBUS_MSG_TEL_STAT              0x2c	/* Telephone status */
#define IKBUS_MSG_TEL_NUMB              0x2d	/* Telephone number */
#define IKBUS_MSG_BUTTON                0x31	/* Button */
#define IKBUS_MSG_VOLUME_CTL            0x32	/* Volume Control */
#define IKBUS_MSG_PART_NUMB_STAT        0x33	/* Part number status */
#define IKBUS_MSG_DSP_EQ_BTN            0x34	/* DSP equaliser button */
#define IKBUS_MSG_CAR_MEM_RESP          0x35	/* Car memory response */
#define IKBUS_MSG_AUDIO_CTL             0x36	/* Audio_control */
#define IKBUS_MSG_SELECT_MENU           0x37	/* Select Menu */
#define IKBUS_MSG_CD_CTL                0x38	/* CD_control */
#define IKBUS_MSG_CD_STAT               0x39	/* CD_status */
#define IKBUS_MSG_RECIRC_AIR_CTL        0x3a	/* Recirculating air control */
#define IKBUS_MSG_RAD_TEL_CTL           0x3b	/* Radio/Telephone control */
#define IKBUS_MSG_DSP_SND_PTRN          0x3c	/* DSP preset sound patterns */
#define IKBUS_MSG_BC_SET_DATA           0x40	/* On-board computer set data */
#define IKBUS_MSG_BC_DATA_REQ           0x41	/* On-board computer data request */
#define IKBUS_MSG_BC_SCROLL             0x42	/* On-board computer scroll */
#define IKBUS_MSG_MONO_DYSP             0x43	/* Mono display */
#define IKBUS_MSG_E46_IKE_TEXT          0x44	/* E46 IKE text */
#define IKBUS_MSG_RAD_STAT_REQ          0x45	/* Radio status request */
#define IKBUS_MSG_LCD_CLER              0x46	/* LCD clear */
#define IKBUS_MSG_BM_STAT               0x47	/* BM Status */
#define IKBUS_MSG_BM_BTN                0x48	/* BM Button */
#define IKBUS_MSG_KNOB                  0x49	/* BM_Knob */
#define IKBUS_MSG_CASSETE_CTL           0x4a	/* Cassette control */
#define IKBUS_MSG_CASSETE_STAT          0x4b	/* Cassette status */
#define IKBUS_MSG_AUDIO_SRC_SEL         0x4e	/* Audio source selection */
#define IKBUS_MSG_RGB_CTL               0x4f	/* RGB control */
#define IKBUS_MSG_CHKCTL_SENS_REQ       0x50	/* Checkcontrol sensor request */
#define IKBUS_MSG_CHKCTL_SENS           0x51	/* Checkcontrol sensors */
#define IKBUS_MSG_TEXT_DYSP_UPDT        0x52	/* Text Display Update */
#define IKBUS_MSG_VEH_DATA_REQ          0x53	/* Vehicle data request */
#define IKBUS_MSG_VEH_DATA_STAT         0x54	/* Vehicle data status */
#define IKBUS_MSG_SERV_INT_DYSP         0x55	/* Service Interval Display */
#define IKBUS_MSG_LIGHT_CTL_REQ         0x56	/* Light control status request */
#define IKBUS_MSG_CHK_CTL_BTN           0x57	/* Check Control button */
#define IKBUS_MSG_HDLGHT_INTVL          0x58	/* Headlight wipe interval */
#define IKBUS_MSG_LIGHT_CTL_STAT        0x59	/* Light control status */
#define IKBUS_MSG_LAMP_STAT_REQ         0x5a	/* Lamp status request */
#define IKBUS_MSG_LAMP_STAT             0x5b	/* Lamp status */
#define IKBUS_MSG_LIGHT_DIM             0x5c	/* Light dimmer */
#define IKBUS_MSG_LIGHT_DIM_TAT_REQ     0x5d	/* Light dimmer status request */
#define IKBUS_MSG_LAM_SENS              0x5e	/* LAM sensor */
#define IKBUS_MSG_INFO_SWAP             0x5f	/* Info swap */
#define IKBUS_MSG_SUSP_CTL_STAT_REQ     0x60	/* Suspension control status request */
#define IKBUS_MSG_SUSP_CTL              0x61	/* Suspension control */
#define IKBUS_MSG_RDS_STAT              0x62	/* RDC status */
#define IKBUS_MSG_MIR_CTL               0x6d	/* Mirror control */
#define IKBUS_MSG_RM_CENT_LOCK_STAT     0x70	/* Remote control central locking status */
#define IKBUS_MSG_RAIN_SENS             0x71	/* Rain sensor status */
#define IKBUS_MSG_RM_CENT_LOCK_CHKCTL   0x72	/* ChkCtrl Remote Central Locking */
#define IKBUS_MSG_IMM_STAT_REQ          0x73	/* Immobiliser status request */
#define IKBUS_MSG_IMM_STAT              0x74	/* Immobiliser status */
#define IKBUS_MSG_WIPER_STAT_REQ        0x75	/* Wiper status request */
#define IKBUS_MSG_CRASH_ALARM           0x76	/* Crash Alarm */
#define IKBUS_MSG_WIPER_STAT            0x77	/* Wiper status */
#define IKBUS_MSG_SSEAT_MEM             0x78	/* Seat Memory */
#define IKBUS_MSG_DOOR_FLAP_STAT_REQ    0x79	/* Doors/flaps status request */
#define IKBUS_MSG_DOOR_FLAP_STAT        0x7a	/* Doors/flaps status */
#define IKBUS_MSG_SUNROOF_STAT          0x7c	/* Sunroof status */
#define IKBUS_MSG_SUNROOF_CTL           0x7d	/* Sunroof control */
#define IKBUS_MSG_COND_COMPR_STAT       0x83	/* Air conditioning compressor status */
#define IKBUS_MSG_HEAT_VENT_STAT        0x86	/* Aux Heating/Vent status */
#define IKBUS_MSG_HEAT_VENT_STAT_REQ    0x87	/* Aux Heating/Vent status request */
#define IKBUS_MSG_HEATER_STAT           0x92	/* Heater status */
#define IKBUS_MSG_HEATER_STAT_REQ       0x93	/* Heater status request */
#define IKBUS_MSG_HEADPHONE_STAT        0x9f	/* Headphone status */
#define IKBUS_MSG_DIA_ACK               0xa0	/* Diagnostic command acknowledged */
#define IKBUS_MSG_CUR_POS_REQ           0xa1	/* Current position request */
#define IKBUS_MSG_CUR_POS               0xa2	/* Current position */
#define IKBUS_MSG_CUR_LOC_REQ           0xa3	/* Current location request */
#define IKBUS_MSG_CUR_LOC               0xa4	/* Current location */
#define IKBUS_MSG_SCREEN_TEXT           0xa5	/* Screen Text */
#define IKBUS_MSG_SPEC_INDICATORS       0xa6	/* Special indicators */
#define IKBUS_MSG_TMC_STAT_REQ          0xa7	/* TMC status request */
#define IKBUS_MSG_TMC_DATA              0xa8	/* TMC data */
#define IKBUS_MSG_PHONE_DATA            0xa9	/* Telephone data */
#define IKBUS_MSG_NAV_CTL               0xaa	/* Nav Control */
#define IKBUS_MSG_RM_CTL_STAT_DATA      0xab	/* Remote control status data */
#define IKBUS_MSG_NG_RAD_STATION_LST    0xd4	/* NG Radio Station list */

/* I/K bus diagnostic commands */
#define IKBUS_DIA_READ_IDENT            0x00	/* Read identity */
#define IKBUS_DIA_WRITE_IDENT           0x01	/* Write identity */
#define IKBUS_DIA_READ_AIS              0x02	/* Read AIS */
#define IKBUS_DIA_WRITE_AIS             0x03	/* Write AIS */
#define IKBUS_DIA_READ_FLT_MEM          0x04	/* Read fault memory */
#define IKBUS_DIA_CLR_FLT_MEM           0x05	/* Clear fault memory */
#define IKBUS_DIA_READ_MEM              0x06	/* Read memory */
#define IKBUS_DIA_WRITE_MEM             0x07	/* Write memory */
#define IKBUS_DIA_READ_COD_DAT          0x08	/* Read coding data */
#define IKBUS_DIA_WRITE_COD_DAT         0x09	/* Write coding data */
#define IKBUS_DIA_REQ_COD_DAT_CHK       0x0a	/* Request coding data checksum */
#define IKBUS_DIA_READ_IO_STAT          0x0b	/* Read IO status */
#define IKBUS_DIA_SET_OI_STAT           0x0c	/* Set IO status */
#define IKBUS_DIA_READ_SYS_ADDR         0x0d	/* Read system address */
#define IKBUS_DIA_READ_TEST_STAMP       0x0e	/* Read test stamp */
#define IKBUS_DIA_SET_TEST_STAMP        0x0f	/* Set test stamp */
#define IKBUS_DIA_CLR_MEM               0x10	/* Clear memory */
#define IKBUS_DIA_READ_OS_BOOT          0x11	/* Read OS boot mode */
#define IKBUS_DIA_RESET_CTL_UNIT        0x12	/* Reset control unit */
#define IKBUS_DIA_READ_FLT_SHDW_MEM     0x14	/* Read fault shadow memory */

#endif	/* _IK_BUS_FRAME_H_ */
