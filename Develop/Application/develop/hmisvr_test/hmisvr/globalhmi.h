#ifndef GLOBALHMI_H
#define GLOBALHMI_H

#include <QtGui>
#include "main_dialog.h"
#include "p02_qrcode_dialog.h"
#include "p03_chargelink_dialog.h"
#include "p04_chargestart_dialog.h"
#include "p05_chargeinfo_dialog.h"
#include "p06_chargeoff_dialog.h"

#include "p07_swingcard_dialog.h"
#include "p08_carpasswd_dialog.h"
#include "p09_chargetypechoose_dialog.h"
#include "p10_chargebytime_dialog.h"
#include "p11_chargebyenergy_dialog.h"
#include "p12_cardpayoff_dialog.h"
#include "p13_cardunlock_dialog.h"

#include "p14_deviceinfo_dialog.h"
#include "p15_chargeprice_dialog.h"
#include "p16_othersevices_dialog.h"
#include "p17_appdownload_dialog.h"
#include "p18_cardinquery_dialog.h"

#include "p19_setpara_passwd_dialog.h"
#include "p20_setpara_sub1_dialog.h"
#include "p21_setpara_sub2_dialog.h"
#include "p22_setpara_sub3_dialog.h"
#include "p23_setpara_sub4_dialog.h"

#include "p24_service_choose_dialog.h"

#include "p25_setpara_sub5_dialog.h"
#include "p26_setpara_sub6_dialog.h"
#include "p27_setpara_sub7_dialog.h"
#include "p28_chargebymoney_dialog.h"

#include "dialog_carderr_reason.h"
#include "dialog_carnumber.h"
#include "dialog_chargerecode.h"
#include "dialog_deviceerror.h"
#include "dialog_diff_card.h"
#include "dialog_endword.h"
#include "dialog_order.h"

#include "dialog_fail_start_reason.h"
#include "dialog_local_charge_recode.h"
#include "dialog_remote_unload_recode.h"

#include "dialog_module_monitor.h"
#include "dialog_module_err_info.h"
#include "dialog_module_output.h"

#include "dialog_swingcard_err.h"
#include "dialog_wait_swingcard.h"
#include "dialog_stoping_stub.h"

#include "dialog_waiting_payoff.h"
#include "dialog_quit_success.h"
#include "dialog_net_err.h"

#include "globalhmi.h"
#include <dlfcn.h>                          //dlopen()
#include "debug.h"                          //debug()
#include "DB_Unity.h"                       //INT8
#include "ioremap.h"                        //PL_HMI_PAGE_INDEX define
#include "gui.h"                            //PAGEINDEX_MAIN
#include "inifile.h"                        //

/*

#include "gui.h"                            //
#include <time.h>                           //sleep()
*/

extern class main_dialog*                    main_dlg;
extern class p02_qrcode_dialog*              p02_dlg;          //
extern class p03_chargelink_dialog*          p03_dlg;          //
extern class p04_chargestart_dialog*         p04_dlg;          //
extern class p05_chargeinfo_dialog*          p05_dlg;          //
extern class p06_chargeoff_dialog*           p06_dlg;          //

extern class p07_swingcard_dialog*           p07_dlg;          //
extern class p08_carpasswd_dialog*           p08_dlg;          //
extern class p09_chargetypechoose_dialog*    p09_dlg;          //
extern class p10_chargebytime_dialog*        p10_dlg;          //
extern class p11_chargebyenergy_dialog*      p11_dlg;          //
extern class p12_cardpayoff_dialog*          p12_dlg;          //
extern class p13_cardunlock_dialog*          p13_dlg;          //

extern class p14_deviceinfo_dialog*          p14_dlg;          //
extern class p15_chargeprice_dialog*         p15_dlg;          //
extern class p16_othersevices_dialog*        p16_dlg;          //
extern class p17_appdownload_dialog*         p17_dlg;          //
extern class p18_cardinquery_dialog*         p18_dlg;          //

extern class p19_setpara_passwd_dialog*      p19_dlg;          //
extern class p20_setpara_sub1_dialog*        p20_dlg;          //
extern class p21_setpara_sub2_dialog*        p21_dlg;          //
extern class p22_setpara_sub3_dialog*        p22_dlg;          //
extern class p23_setpara_sub4_dialog*        p23_dlg;          //
extern class p24_service_choose_dialog*      p24_dlg;
extern class p25_setpara_sub5_dialog*        p25_dlg;
extern class p26_setpara_sub6_dialog*        p26_dlg;
extern class p27_setpara_sub7_dialog*        p27_dlg;
extern class p28_chargebymoney_dialog*       p28_dlg;

extern class dialog_carnumber*              carnumber_dlg;              //
extern class dialog_diff_card*               diff_card_dlg;             //
extern class dialog_deviceerror*             deviceerror_dlg;           //
extern class dialog_order*                   order_dlg;                 //
extern class dialog_carderr_reason*          carderr_reason_dlg;        //
extern class dialog_endword*                 endword_dlg;               //
extern class dialog_chargerecode*            chargerecode_dlg;          //

extern class dialog_fail_start_reason*       fail_start_reason_dlg;
extern class dialog_local_charge_recode*     local_charge_recode_dlg;
extern class dialog_remote_unload_recode*    remote_unload_recode_dlg;

extern class dialog_module_monitor*          module_monitor_dlg;
extern class dialog_module_err_info*         module_err_info_dlg;
extern class dialog_module_output*           module_output_dlg;

extern class dialog_swingcard_err*           swingcard_err_dlg;
extern class dialog_wait_swingcard *         wait_swingcard_dlg;
extern class dialog_stoping_stub*            stoping_stub_dlg;

extern class dialog_waiting_payoff*          waiting_payoff_dlg;
extern class dialog_quit_success *           quit_success_dlg;
extern class dialog_net_err*                 net_err_dlg;

//#define         BUFFERLEN_32         32
#define         HMI_LOG_LEV          4
#define         SETPAGE_FLUSH_TIMER  50 //20
#define         MAX_PASSWD_LEN       16

extern UINT8    hmi_button_dn_num;
extern UINT8    hmi_page_index;
extern INT8     hmilev;                 //debug()

extern void     ( *API_DB_Initial ) ( void );
extern INT16    ( *API_Read_DB_Nbyte ) ( UINT32 label,void* Data,UINT16 n );
extern INT16    ( *API_Write_DB_Nbyte ) ( UINT32 label,void* Data,UINT16 n );
extern int      Inibuf_init ( char* storebuf,int storebuf_len,const char* setvalue );
extern void     timedelay_msec ( unsigned int msec );

extern int      language_num;

extern char     Section_name[BUFFERLEN_32];
extern char     Key_name[BUFFERLEN_32];
extern char     Tempor_name[256];

extern char     Set_paswd[17];
extern char     phonenum[25];
extern char     phonenum_2[25];
extern char     code[25];
extern char     websize[100];
extern char     appdownloadas[100];

extern const char*    config_path;

extern INT8     main_card_sn[16+1];


#endif
