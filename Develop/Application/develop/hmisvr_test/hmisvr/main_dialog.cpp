#include "main_dialog.h"
#include "ui_main_dialog.h"
#include "globalhmi.h"

main_dialog::main_dialog ( QWidget* parent ) :
	QDialog ( parent ),
	ui ( new Ui::main_dialog )
{
	ui->setupUi ( this );
	init_main_dlg();
}

main_dialog::~main_dialog()
{
	delete ui;
}

static int add_num = 0;
static int Timer = 0;
static int setframe_screensr_flag = 1;      // 1 => not show the screensaver ; 2 => show

static int add_network_num = 0;
static int click_screen_flag = 0;

void main_dialog::changeEvent ( QEvent* e )
{
	QDialog::changeEvent ( e );
	switch ( e->type() )
	{
		case QEvent::LanguageChange:
			ui->retranslateUi ( this );
			break;
		default:
			break;
	}
}

void main_dialog::init_main_dlg()
{
	add_num=0;
	Timer=5;                        //1  // 30s end

	ui->frame_ss1->move ( 0,800 );
	ui->frame_ss2->move ( 0,800 );
	ui->frame_main->move ( 0,0 );

	timer = new QTimer ( this );

	connect ( timer,SIGNAL ( timeout() ),this,SLOT ( timer_handle() ) );
	timer->start ( SETPAGE_FLUSH_TIMER );
	init_database();
}

void main_dialog::init_database()
{

	void* fp;
	DebugMsg ( hmilev,8,"init_database\n" );
	if ( ( fp=dlopen ( "libdatabase.so",RTLD_LAZY ) ) ==NULL )
	{
		fputs ( dlerror(),stderr );
		printf ( "\n" );
		printf ( "can not open the library\n" );
		exit ( 1 );
	}
	API_Read_DB_Nbyte = ( INT16 ( * ) ( UINT32,void*,UINT16 ) ) dlsym ( fp,"API_Read_DB_Nbyte" );
	API_Write_DB_Nbyte = ( INT16 ( * ) ( UINT32,void*,UINT16 ) ) dlsym ( fp,"API_Write_DB_Nbyte" );
	API_DB_Initial = ( void ( * ) () ) dlsym ( fp,"API_DB_Initial" );

	API_DB_Initial();

	UINT8 data = PAGEINDEX_MAIN;
	API_Write_DB_Nbyte ( PL_HMI_PAGE_INDEX, ( INT8* ) ( &data ),sizeof ( data ) );
	API_Write_DB_Nbyte ( PL_HMI_PAGE_INDEX_CUR, ( INT8* ) ( &data ),sizeof ( data ) );

	data = NO_BUTTON;
	API_Write_DB_Nbyte ( PL_HMI_BUTTON_DOWN, ( INT8* ) ( &data ),sizeof ( data ) );

	//flag of net work state
	int net_flg;
	net_flg=0;
	int temp_net;
	temp_net=0;
	Inibuf_init ( Section_name,BUFFERLEN_32,"SYSTEMINFOR" );
	Inibuf_init ( Key_name,BUFFERLEN_32,"networkservice" );
	net_flg=read_profile_int ( Section_name,Key_name,temp_net,config_path );
	API_Write_DB_Nbyte ( PL_NEEDNETWORK_FLAG, ( BOOL* ) ( &net_flg ),sizeof ( net_flg ) );
}

void main_dialog::start_timer()
{
	//printf ( "****************** start_timer int main_dialog \n" );
	timer->start ( SETPAGE_FLUSH_TIMER );
}

void main_dialog::timer_handle()
{
	char buf_code[100]= {0};

	memset ( phonenum,0,sizeof ( phonenum ) );
	memset ( Tempor_name,0,sizeof ( Tempor_name ) );
	Inibuf_init ( Section_name,BUFFERLEN_32,"SYSTEMINFOR" );
	Inibuf_init ( Key_name,BUFFERLEN_32,"oneemergencycall" );
	if ( read_profile_string ( Section_name, Key_name, Tempor_name, INIFILE_LININBUF_SIZE,"",config_path ) )
	{
		memcpy ( phonenum,Tempor_name,sizeof ( phonenum ) );
	}
	else
	{
		memset ( phonenum,0,sizeof ( phonenum ) );
	}

	ui->label_phone->setText ( phonenum );

	memset ( websize,0,sizeof ( websize ) );
	memset ( Tempor_name,0,sizeof ( Tempor_name ) );
	Inibuf_init ( Section_name,BUFFERLEN_32,"SYSTEMINFOR" );
	Inibuf_init ( Key_name,BUFFERLEN_32,"websize" );
	if ( read_profile_string ( Section_name, Key_name, Tempor_name, INIFILE_LININBUF_SIZE,"",config_path ) )
	{
		memcpy ( websize,Tempor_name,sizeof ( websize ) );
	}
	else
	{
		memset ( websize,0,sizeof ( websize ) );
	}

	snprintf ( buf_code,100,"/usr/APP/qrencode -o app.png -s 4 %s",websize );

	system ( buf_code );
	memset ( buf_code,0,100 );
	snprintf ( buf_code,100,"pkill qrencode" );
	system ( buf_code );

	QPixmap qrencode_pixmap ( QString ( "/usr/APP/app.png" ) );
	qrencode_pixmap = qrencode_pixmap.scaled ( ui->label_qrencode->width(), ui->label_qrencode->height(),
	                                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation );

	ui->label_qrencode->setPixmap ( qrencode_pixmap );

	check_hmi_page_index();

	ui->label_sysdate->setText ( QDateTime::currentDateTime().toString ( "yyyy-MM-dd" ) );
	ui->label_systime->setText ( QTime::currentTime().toString ( "hh:mm:ss" ) );

	ui->pbtn_card_charge->setStyleSheet ( "QPushButton{border-image: url(:/new/prefix/pic_resource/maindialog/card_charge_01.png);}\
                                QPushButton:pressed{border-image: url(:/new/prefix/pic_resource/maindialog/card_charge_02.png);}" );
	ui->pbtn_card_inquiry->setStyleSheet ( "QPushButton{border-image: url(:/new/prefix/pic_resource/maindialog/card_inquiry_01.png);}\
                                QPushButton:pressed{border-image: url(:/new/prefix/pic_resource/maindialog/card_inquiry_02.png);}" );
	ui->label_net->setStyleSheet ( "border-image: url(:/new/prefix/pic_resource/otherwidget/netstat_bad.png);" );

	int net_flg2;
	API_Read_DB_Nbyte ( PL_NETWORKLINKSTATE_FLAG, ( BOOL* ) ( &net_flg2 ),sizeof ( net_flg2 ) );
	if ( net_flg2 )
	{
		add_network_num=0;
		ui->label_net->setStyleSheet ( "border-image: url(:/new/prefix/pic_resource/otherwidget/netstat_good.png);" );
	}
	else
	{
		add_network_num++;
		if ( add_network_num>=750 )
		{
			ui->label_net->setStyleSheet ( "border-image: url(:/new/prefix/pic_resource/otherwidget/netstat_bad.png);" );
			add_network_num=0;
		}
	}

}
void main_dialog::check_hmi_page_index()
{
	if ( setframe_screensr_flag == 1 )
	{
		add_num++;
		if ( add_num==20 )
		{
			add_num=0;
			Timer--;
			if ( Timer<=0 )
			{
				Timer = 5;

				if ( click_screen_flag == 1 )
				{
					click_screen_flag = 0;
				}

				if ( hmi_page_index==PAGEINDEX_MAIN )
				{
					setframe_screensr1_up();
				}

			}
		}
	}
	else
	{
		//printf ( "****************** not show the screensaver\n" );
	}

	API_Read_DB_Nbyte ( PL_HMI_PAGE_INDEX, ( INT8* ) ( &hmi_page_index ),sizeof ( hmi_page_index ) );

	if ( hmi_page_index==PAGEINDEX_MAIN )
	{
		if ( p02_dlg )
		{
			delete p02_dlg;
			p02_dlg = NULL;
		}
		if ( p03_dlg )
		{
			delete p03_dlg;
			p03_dlg = NULL;
		}
		if ( p04_dlg )
		{
			delete p04_dlg;
			p04_dlg = NULL;
		}
		if ( p05_dlg )
		{
			delete p05_dlg;
			p05_dlg = NULL;
		}
		if ( p06_dlg )
		{
			delete p06_dlg;
			p06_dlg = NULL;
		}
		if ( p07_dlg )
		{
			delete p07_dlg;
			p07_dlg = NULL;
		}
		if ( p08_dlg )
		{
			delete p08_dlg;
			p08_dlg = NULL;
		}
		if ( p09_dlg )
		{
			delete p09_dlg;
			p09_dlg = NULL;
		}
		if ( p10_dlg )
		{
			delete p10_dlg;
			p10_dlg = NULL;
		}
		if ( p11_dlg )
		{
			delete p11_dlg;
			p11_dlg = NULL;
		}
		if ( p12_dlg )
		{
			delete p12_dlg;
			p12_dlg = NULL;
		}
		if ( p13_dlg )
		{
			delete p13_dlg;
			p13_dlg = NULL;
		}
		if ( p14_dlg )
		{
			delete p14_dlg;
			p14_dlg = NULL;
		}
		if ( p15_dlg )
		{
			delete p15_dlg;
			p15_dlg = NULL;
		}
		if ( p16_dlg )
		{
			delete p16_dlg;
			p16_dlg = NULL;
		}
		if ( p17_dlg )
		{
			delete p17_dlg;
			p17_dlg = NULL;
		}
		if ( p18_dlg )
		{
			delete p18_dlg;
			p18_dlg = NULL;
		}
		if ( p19_dlg )
		{
			delete p19_dlg;
			p19_dlg = NULL;
		}
		if ( p20_dlg )
		{
			delete p20_dlg;
			p20_dlg = NULL;
		}
		if ( p21_dlg )
		{
			delete p21_dlg;
			p21_dlg = NULL;
		}
		if ( p22_dlg )
		{
			delete p22_dlg;
			p22_dlg = NULL;
		}
		if ( p23_dlg )
		{
			delete p23_dlg;
			p23_dlg = NULL;
		}
		if ( p24_dlg )
		{
			delete p24_dlg;
			p24_dlg = NULL;
		}
		if ( p25_dlg )
		{
			delete p25_dlg;
			p25_dlg = NULL;
		}
		if ( p26_dlg )
		{
			delete p26_dlg;
			p26_dlg = NULL;
		}
		if ( p27_dlg )
		{
			delete p27_dlg;
			p27_dlg = NULL;
		}
		if ( p28_dlg )
		{
			delete p28_dlg;
			p28_dlg = NULL;
		}
		if ( carnumber_dlg )
		{
			delete carnumber_dlg;
			carnumber_dlg = NULL;
		}
		if ( diff_card_dlg )
		{
			delete diff_card_dlg;
			diff_card_dlg = NULL;
		}
		if ( deviceerror_dlg )
		{
			delete deviceerror_dlg;
			deviceerror_dlg = NULL;
		}
		if ( order_dlg )
		{
			delete order_dlg;
			order_dlg = NULL;
		}
		if ( carderr_reason_dlg )
		{
			delete carderr_reason_dlg;
			carderr_reason_dlg = NULL;
		}
		if ( diff_card_dlg )
		{
			delete diff_card_dlg;
			diff_card_dlg = NULL;
		}
		if ( endword_dlg )
		{
			delete endword_dlg;
			endword_dlg = NULL;
		}
		if ( chargerecode_dlg )
		{
			delete chargerecode_dlg;
			chargerecode_dlg = NULL;
		}
		if ( fail_start_reason_dlg )
		{
			delete fail_start_reason_dlg;
			fail_start_reason_dlg = NULL;
		}
		if ( local_charge_recode_dlg )
		{
			delete local_charge_recode_dlg;
			local_charge_recode_dlg = NULL;
		}
		if ( remote_unload_recode_dlg )
		{
			delete remote_unload_recode_dlg;
			remote_unload_recode_dlg = NULL;
		}
		if ( module_monitor_dlg )
		{
			delete module_monitor_dlg;
			module_monitor_dlg = NULL;
		}
		if ( module_err_info_dlg )
		{
			delete module_err_info_dlg;
			module_err_info_dlg = NULL;
		}
		if ( module_output_dlg )
		{
			delete module_output_dlg;
			module_output_dlg = NULL;
		}
        if ( swingcard_err_dlg)
		{
			delete swingcard_err_dlg;
			swingcard_err_dlg = NULL;
		}
		if ( wait_swingcard_dlg)
		{
			delete wait_swingcard_dlg;
			wait_swingcard_dlg = NULL;
		}
		if ( stoping_stub_dlg)
		{
			delete stoping_stub_dlg;
			stoping_stub_dlg = NULL;
		}
		if ( waiting_payoff_dlg)
		{
			delete waiting_payoff_dlg;
			waiting_payoff_dlg = NULL;
		}
		if ( quit_success_dlg)
		{
			delete quit_success_dlg;
			quit_success_dlg = NULL;
		}
        if ( net_err_dlg)
		{
			delete net_err_dlg;
			net_err_dlg = NULL;
		}

	}
	else if ( hmi_page_index==PAGEINDEX_QRCODE )
	{
		printf ( "@@@@Go to PAGEINDEX_QRCODE!\r\n" );
		call_p02_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CHARGELINK )
	{
		printf ( "@@@@Go to PAGEINDEX_CHARGELINK!\r\n" );
		call_p03_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CHARGESTART )
	{
		printf ( "@@@@Go to PAGEINDEX_CHARGESTART!\r\n" );
		call_p04_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CHARGEINFO )
	{
		printf ( "@@@@Go to PAGEINDEX_CHARGEINFO!\r\n" );
		call_p05_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CHARGEOFF )
	{
		printf ( "@@@@Go to PAGEINDEX_CHARGEOFF!\r\n" );
		call_p06_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_SWINGCARD )
	{
		printf ( "@@@@Go to PAGEINDEX_SWINGCARD!\r\n" );
		call_p07_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CARDPASSWD )
	{
		printf ( "@@@@Go to PAGEINDEX_CARDPASSWD!\r\n" );
		call_p08_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CHARGETYPECHOOSE )
	{
		printf ( "@@@@Go to PAGEINDEX_CHARGETYPECHOOSE!\r\n" );
		call_p09_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CHARGEBYTIME )
	{
		printf ( "@@@@Go to PAGEINDEX_CHARGEBYTIME!\r\n" );
		call_p10_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CHARGEBYENERGY )
	{
		printf ( "@@@@Go to PAGEINDEX_CHARGEBYENERGY!\r\n" );
		call_p11_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CARDPAYOFF )
	{
		printf ( "@@@@Go to PAGEINDEX_CARDPAYOFF!\r\n" );
		call_p12_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CARDUNLOCK )
	{
		printf ( "@@@@Go to PAGEINDEX_CARDUNLOCK!\r\n" );
		call_p13_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_DEVICEINFO )
	{
		printf ( "@@@@Go to PAGEINDEX_DEVICEINFO!\r\n" );
		call_p14_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CHARGEPRICE )
	{
		printf ( "@@@@Go to PAGEINDEX_CHARGEPRICE!\r\n" );
		call_p15_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_OTHERSEVICES )
	{
		printf ( "@@@@Go to PAGEINDEX_OTHERSEVICES!\r\n" );
		call_p16_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_APPDOWNLOAD )
	{
		printf ( "@@@@Go to PAGEINDEX_APPDOWNLOAD!\r\n" );
		call_p17_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CARDINQUERY )
	{
		printf ( "@@@@Go to PAGEINDEX_CARDINQUERY!\r\n" );
		call_p18_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_SETPARAPASSWD )
	{
		printf ( "@@@@Go to PAGEINDEX_SETPARAPASSWD!\r\n" );
		call_p19_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_SETPARASUB1 )
	{
		printf ( "@@@@Go to PAGEINDEX_SETPARASUB1!\r\n" );
		call_p20_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_SETPARASUB2 )
	{
		printf ( "@@@@Go to PAGEINDEX_SETPARASUB2!\r\n" );
		call_p21_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_SETPARASUB3 )
	{
		printf ( "@@@@Go to PAGEINDEX_SETPARASUB3!\r\n" );
		call_p22_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_SETPARASUB4 )
	{
		printf ( "@@@@Go to PAGEINDEX_SETPARASUB4!\r\n" );
		call_p23_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_SERVICECHOOSE )
	{
		printf ( "@@@@Go to PAGEINDEX_SERVICECHOOSE!\r\n" );
		call_p24_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_SETPARASUB5 )
	{
		printf ( "@@@@Go to PAGEINDEX_SETPARASUB5!\r\n" );
		call_p25_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_SETPARASUB6 )
	{
		printf ( "@@@@Go to PAGEINDEX_SETPARASUB2!\r\n" );
		call_p26_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_SETPARASUB7 )
	{
		printf ( "@@@@Go to PAGEINDEX_SETPARASUB7!\r\n" );
		call_p27_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CHARGEBYMONEY )
	{
		printf ( "@@@@Go to PAGEINDEX_CARDNUMBER!\r\n" );
		call_p28_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CARDNUMBER )
	{
		printf ( "@@@@Go to PAGEINDEX_CARDNUMBER!\r\n" );
		call_carnumber_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_DIFFCARD )
	{
		printf ( "@@@@Go to PAGEINDEX_DIFFCARD!\r\n" );
		call_diff_card_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_DEVICEERROR )
	{
		printf ( "@@@@Go to PAGEINDEX_DEVICEERROR!\r\n" );
		call_deviceerror_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_ORDER )
	{
		printf ( "@@@@Go to PAGEINDEX_ORDER!\r\n" );
		call_order_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CARDERRREASON )
	{
		printf ( "@@@@Go to PAGEINDEX_CARDERRREASON!\r\n" );
		call_carderr_reason_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_ENDWORD )
	{
		printf ( "@@@@Go to PAGEINDEX_ENDWORD!\r\n" );
		call_endword_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_FAIL_START_RESON )
	{
		printf ( "@@@@Go to PAGEINDEX_FAIL_START_RESON!\r\n" );
		call_fail_start_reason_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_LOCAL_CHARGE_RECODE )
	{
		printf ( "@@@@Go to PAGEINDEX_LOCAL_CHARGE_RECODE!\r\n" );
		call_local_charge_recode_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_REMOTE_UNLOAD_RECODE )
	{
		printf ( "@@@@Go to PAGEINDEX_REMOTE_UNLOAD_RECODE!\r\n" );
		call_remote_unload_recode_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_MODULE_MONITOR )
	{
		printf ( "@@@@Go to PAGEINDEX_MODULE_MONITOR!\r\n" );
		call_module_monitor_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_MODULE_ERR_INFO )
	{
		printf ( "@@@@Go to PAGEINDEX_MODULE_ERR_INFO!\r\n" );
		call_module_err_info_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_MODULE_OUTPUT )
	{
		printf ( "@@@@Go to PAGEINDEX_MODULE_OUTPUT!\r\n" );
		call_module_output_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_CHARGERECODE )
	{
		printf ( "@@@@Go to PAGEINDEX_CHARGERECODE!\r\n" );
		call_chargerecode_dlg();
	}
    else if ( hmi_page_index==PAGEINDEX_SWINGCARD_ERR)
	{
		printf ( "@@@@Go to PAGEINDEX_SWINGCARD_ERR!\r\n" );
		call_swingcard_err_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_WAIT_SWINGCARD)
	{
		printf ( "@@@@Go to PAGEINDEX_WAIT_SWINGCARD!\r\n" );
		call_wait_swingcard_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_STOPING_STUB)
	{
		printf ( "@@@@Go to PAGEINDEX_STOPING_STUB!\r\n" );
		call_stoping_stub_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_WAITING_PAYOFF)
	{
		printf ( "@@@@Go to PAGEINDEX_WAITING_PAYOFF!\r\n" );
		call_waiting_payoff_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_QUIT_SUCCESS)
	{
		printf ( "@@@@Go to PAGEINDEX_QUIT_SUCCESS!\r\n" );
		call_quit_success_dlg();
	}
	else if ( hmi_page_index==PAGEINDEX_NET_ERR)
	{
		printf ( "@@@@Go to PAGEINDEX_NET_ERR!\r\n" );
		call_net_err_dlg();
	}
	else
	{
		ErrMsg ( "hmi_page_index error[%d]*******%s,%d\n",hmi_page_index,__FILE__,__LINE__ );
	}

}

void main_dialog::call_p02_dlg()
{
	timer->stop();
	if ( p02_dlg )
	{
		delete p02_dlg;
		p02_dlg = NULL;
	}
	p02_dlg = new  p02_qrcode_dialog;
	p02_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_p02dlg_timer() ),p02_dlg,SLOT ( init_page_dlg() ) );
	connect ( p02_dlg,SIGNAL ( signal_exit_p02dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p02dlg_timer();
	p02_dlg->exec();
}

void main_dialog::call_p03_dlg()
{
	timer->stop();
	if ( p03_dlg )
	{
		delete p03_dlg;
		p03_dlg = NULL;
	}
	p03_dlg = new  p03_chargelink_dialog;
	p03_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_p03dlg_timer() ),p03_dlg,SLOT ( init_page_dlg() ) );
	connect ( p03_dlg,SIGNAL ( signal_exit_p03dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p03dlg_timer();
	p03_dlg->exec();
}

void main_dialog::call_p04_dlg()
{
	timer->stop();
	if ( p04_dlg )
	{
		delete p04_dlg;
		p04_dlg = NULL;
	}
	p04_dlg = new  p04_chargestart_dialog;
	p04_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_p04dlg_timer() ),p04_dlg,SLOT ( init_page_dlg() ) );
	connect ( p04_dlg,SIGNAL ( signal_exit_p04dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p04dlg_timer();
	p04_dlg->exec();
}

void main_dialog::call_p05_dlg()
{
	timer->stop();
	if ( p05_dlg )
	{
		delete p05_dlg;
		p05_dlg = NULL;
	}
	p05_dlg = new  p05_chargeinfo_dialog;
	p05_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p05dlg_timer() ),p05_dlg,SLOT ( init_page_dlg() ) );
	connect ( p05_dlg,SIGNAL ( signal_exit_p05dlg() ),this,SLOT ( start_timer() ) );

	emit signal_enable_p05dlg_timer();
	p05_dlg->exec();
}

void main_dialog::call_p06_dlg()
{
	timer->stop();
	if ( p06_dlg )
	{
		delete p06_dlg;
		p06_dlg = NULL;
	}
	p06_dlg = new  p06_chargeoff_dialog;
	p06_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p06dlg_timer() ),p06_dlg,SLOT ( init_page_dlg() ) );
	connect ( p06_dlg,SIGNAL ( signal_exit_p06dlg() ),this,SLOT ( start_timer() ) );

	emit signal_enable_p06dlg_timer();
	p06_dlg->exec();
}

void main_dialog::call_p07_dlg()
{
	timer->stop();
	if ( p07_dlg )
	{
		delete p07_dlg;
		p07_dlg = NULL;
	}
	p07_dlg = new  p07_swingcard_dialog;
	p07_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p07dlg_timer() ),p07_dlg,SLOT ( init_page_dlg() ) );
	connect ( p07_dlg,SIGNAL ( signal_exit_p07dlg() ),this,SLOT ( start_timer() ) );

	emit signal_enable_p07dlg_timer();
	p07_dlg->exec();
}

void main_dialog::call_p08_dlg()
{
	timer->stop();
	if ( p08_dlg )
	{
		delete p08_dlg;
		p08_dlg = NULL;
	}
	p08_dlg = new  p08_carpasswd_dialog;
	p08_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p08dlg_timer() ),p08_dlg,SLOT ( init_page_dlg() ) );
	connect ( p08_dlg,SIGNAL ( signal_exit_p08dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p08dlg_timer();
	p08_dlg->exec();
}

void main_dialog::call_p09_dlg()
{
	timer->stop();
	if ( p09_dlg )
	{
		delete p09_dlg;
		p09_dlg = NULL;
	}
	p09_dlg = new  p09_chargetypechoose_dialog;
	p09_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p09dlg_timer() ),p09_dlg,SLOT ( init_page_dlg() ) );
	connect ( p09_dlg,SIGNAL ( signal_exit_p09dlg() ),this,SLOT ( start_timer() ) );

	emit signal_enable_p09dlg_timer();
	p09_dlg->exec();
}

void main_dialog::call_p10_dlg()
{
	timer->stop();
	if ( p10_dlg )
	{
		delete p10_dlg;
		p10_dlg = NULL;
	}
	p10_dlg = new  p10_chargebytime_dialog;
	p10_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p10dlg_timer() ),p10_dlg,SLOT ( init_page_dlg() ) );
	connect ( p10_dlg,SIGNAL ( signal_exit_p10dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p10dlg_timer();
	p10_dlg->exec();
}

void main_dialog::call_p11_dlg()
{
	timer->stop();
	if ( p11_dlg )
	{
		delete p11_dlg;
		p11_dlg = NULL;
	}
	p11_dlg = new  p11_chargebyenergy_dialog;
	p11_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p11dlg_timer() ),p11_dlg,SLOT ( init_page_dlg() ) );
	connect ( p11_dlg,SIGNAL ( signal_exit_p11dlg() ),this,SLOT ( start_timer() ) );

	emit signal_enable_p11dlg_timer();
	p11_dlg->exec();
}

void main_dialog::call_p12_dlg()
{
	timer->stop();
	if ( p12_dlg )
	{
		delete p12_dlg;
		p12_dlg = NULL;
	}
	p12_dlg = new  p12_cardpayoff_dialog;
	p12_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p12dlg_timer() ),p12_dlg,SLOT ( init_page_dlg() ) );
	connect ( p12_dlg,SIGNAL ( signal_exit_p12dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p12dlg_timer();
	p12_dlg->exec();
}

void main_dialog::call_p13_dlg()
{
	timer->stop();
	if ( p13_dlg )
	{
		delete p13_dlg;
		p13_dlg = NULL;
	}
	p13_dlg = new  p13_cardunlock_dialog;
	p13_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p13dlg_timer() ),p13_dlg,SLOT ( init_page_dlg() ) );
	connect ( p13_dlg,SIGNAL ( signal_exit_p13dlg() ),this,SLOT ( start_timer() ) );

	emit signal_enable_p13dlg_timer();
	p13_dlg->exec();
}

void main_dialog::call_p14_dlg()
{
	timer->stop();
	if ( p14_dlg )
	{
		delete p14_dlg;
		p14_dlg = NULL;
	}
	p14_dlg = new  p14_deviceinfo_dialog;
	p14_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p14dlg_timer() ),p14_dlg,SLOT ( init_page_dlg() ) );
	connect ( p14_dlg,SIGNAL ( signal_exit_p14dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p14dlg_timer();
	p14_dlg->exec();
}

void main_dialog::call_p15_dlg()
{
	timer->stop();
	if ( p15_dlg )
	{
		delete p15_dlg;
		p15_dlg = NULL;
	}
	p15_dlg = new  p15_chargeprice_dialog;
	p15_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p15dlg_timer() ),p15_dlg,SLOT ( init_page_dlg() ) );
	connect ( p15_dlg,SIGNAL ( signal_exit_p15dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p15dlg_timer();
	p15_dlg->exec();
}

void main_dialog::call_p16_dlg()
{
	timer->stop();
	if ( p16_dlg )
	{
		delete p16_dlg;
		p16_dlg = NULL;
	}
	p16_dlg = new  p16_othersevices_dialog;
	p16_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p16dlg_timer() ),p16_dlg,SLOT ( init_page_dlg() ) );
	connect ( p16_dlg,SIGNAL ( signal_exit_p16dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p16dlg_timer();
	p16_dlg->exec();
}

void main_dialog::call_p17_dlg()
{
	timer->stop();
	if ( p17_dlg )
	{
		delete p17_dlg;
		p17_dlg = NULL;
	}
	p17_dlg = new  p17_appdownload_dialog;
	p17_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p17dlg_timer() ),p17_dlg,SLOT ( init_page_dlg() ) );
	connect ( p17_dlg,SIGNAL ( signal_exit_p17dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p17dlg_timer();
	p17_dlg->exec();
}

void main_dialog::call_p18_dlg()
{
	timer->stop();
	if ( p18_dlg )
	{
		delete p18_dlg;
		p18_dlg = NULL;
	}
	p18_dlg = new  p18_cardinquery_dialog;
	p18_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p18dlg_timer() ),p18_dlg,SLOT ( init_page_dlg() ) );
	connect ( p18_dlg,SIGNAL ( signal_exit_p18dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p18dlg_timer();
	p18_dlg->exec();
}

void main_dialog::call_p19_dlg()
{
	timer->stop();
	if ( p19_dlg )
	{
		delete p19_dlg;
		p19_dlg = NULL;
	}
	p19_dlg = new  p19_setpara_passwd_dialog;
	p19_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p19dlg_timer() ),p19_dlg,SLOT ( init_page_dlg() ) );
	connect ( p19_dlg,SIGNAL ( signal_exit_p19dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p19dlg_timer();
	p19_dlg->exec();
}

void main_dialog::call_p20_dlg()
{
	timer->stop();
	if ( p20_dlg )
	{
		delete p20_dlg;
		p20_dlg = NULL;
	}
	p20_dlg = new  p20_setpara_sub1_dialog;
	p20_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p20dlg_timer() ),p20_dlg,SLOT ( init_page_dlg() ) );
	connect ( p20_dlg,SIGNAL ( signal_exit_p20dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p20dlg_timer();
	p20_dlg->exec();
}

void main_dialog::call_p21_dlg()
{
	timer->stop();
	if ( p21_dlg )
	{
		delete p21_dlg;
		p21_dlg = NULL;
	}
	p21_dlg = new  p21_setpara_sub2_dialog;
	p21_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p21dlg_timer() ),p21_dlg,SLOT ( init_page_dlg() ) );
	connect ( p21_dlg,SIGNAL ( signal_exit_p21dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p21dlg_timer();
	p21_dlg->exec();
}

void main_dialog::call_p22_dlg()
{
	timer->stop();
	if ( p22_dlg )
	{
		delete p22_dlg;
		p22_dlg = NULL;
	}
	p22_dlg = new  p22_setpara_sub3_dialog;
	p22_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p22dlg_timer() ),p22_dlg,SLOT ( init_page_dlg() ) );
	connect ( p22_dlg,SIGNAL ( signal_exit_p22dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p21dlg_timer();
	p22_dlg->exec();
}

void main_dialog::call_p23_dlg()
{
	timer->stop();
	if ( p23_dlg )
	{
		delete p23_dlg;
		p23_dlg = NULL;
	}
	p23_dlg = new  p23_setpara_sub4_dialog;
	p23_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p23dlg_timer() ),p23_dlg,SLOT ( init_page_dlg() ) );
	connect ( p23_dlg,SIGNAL ( signal_exit_p23dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p23dlg_timer();
	p23_dlg->exec();
}

void main_dialog::call_p24_dlg()
{
	timer->stop();
	if ( p24_dlg )
	{
		delete p24_dlg;
		p24_dlg = NULL;
	}
	p24_dlg = new  p24_service_choose_dialog;
	p24_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p24dlg_timer() ),p24_dlg,SLOT ( init_page_dlg() ) );
	connect ( p24_dlg,SIGNAL ( signal_exit_p24dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p24dlg_timer();
	p24_dlg->exec();
}

void main_dialog::call_p25_dlg()
{
	timer->stop();
	if ( p25_dlg )
	{
		delete p25_dlg;
		p25_dlg = NULL;
	}
	p25_dlg = new  p25_setpara_sub5_dialog;
	p25_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p25dlg_timer() ),p25_dlg,SLOT ( init_page_dlg() ) );
	connect ( p25_dlg,SIGNAL ( signal_exit_p25dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p25dlg_timer();
	p25_dlg->exec();
}

void main_dialog::call_p26_dlg()
{
	timer->stop();
	if ( p26_dlg )
	{
		delete p26_dlg;
		p26_dlg = NULL;
	}
	p26_dlg = new  p26_setpara_sub6_dialog;
	p26_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p26dlg_timer() ),p26_dlg,SLOT ( init_page_dlg() ) );
	connect ( p26_dlg,SIGNAL ( signal_exit_p26dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p26dlg_timer();
	p26_dlg->exec();
}

void main_dialog::call_p27_dlg()
{
	timer->stop();
	if ( p27_dlg )
	{
		delete p27_dlg;
		p27_dlg = NULL;
	}
	p27_dlg = new  p27_setpara_sub7_dialog;
	p27_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p27dlg_timer() ),p27_dlg,SLOT ( init_page_dlg() ) );
	connect ( p27_dlg,SIGNAL ( signal_exit_p27dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p27dlg_timer();
	p27_dlg->exec();
}

void main_dialog::call_p28_dlg()
{
	timer->stop();
	if ( p28_dlg )
	{
		delete p28_dlg;
		p28_dlg = NULL;
	}
	p28_dlg = new  p28_chargebymoney_dialog;
	p28_dlg->setWindowFlags ( Qt::FramelessWindowHint );

	connect ( this,SIGNAL ( signal_enable_p28dlg_timer() ),p28_dlg,SLOT ( init_page_dlg() ) );
	connect ( p28_dlg,SIGNAL ( signal_exit_p28dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_p28dlg_timer();
	p28_dlg->exec();
}

void main_dialog::call_carnumber_dlg()
{
	timer->stop();
	if ( carnumber_dlg )
	{
		delete carnumber_dlg;
		carnumber_dlg = NULL;
	}
	carnumber_dlg = new  dialog_carnumber;
	carnumber_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_cardnumber_timer() () ),carnumber_dlg,SLOT ( init_page_dlg() ) );
	connect ( carnumber_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_cardnumber_timer();
	carnumber_dlg->exec();
}

void main_dialog::call_diff_card_dlg()
{
	timer->stop();
	if ( diff_card_dlg )
	{
		delete diff_card_dlg;
		diff_card_dlg = NULL;
	}
	diff_card_dlg = new  dialog_diff_card;
	diff_card_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_diffcard_timer() ),diff_card_dlg,SLOT ( init_page_dlg() ) );
	connect ( diff_card_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_diffcard_timer();
	diff_card_dlg->exec();
}
void main_dialog::call_deviceerror_dlg()
{
	timer->stop();
	if ( deviceerror_dlg )
	{
		delete deviceerror_dlg;
		deviceerror_dlg = NULL;
	}
	deviceerror_dlg = new  dialog_deviceerror;
	deviceerror_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_diffcard_timer() ),deviceerror_dlg,SLOT ( init_page_dlg() ) );
	connect ( deviceerror_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_diffcard_timer();
	deviceerror_dlg->exec();
}

void main_dialog::call_order_dlg()
{
	timer->stop();
	if ( order_dlg )
	{
		delete order_dlg;
		order_dlg = NULL;
	}
	order_dlg = new  dialog_order;
	order_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_order_timer() ),order_dlg,SLOT ( init_page_dlg() ) );
	connect ( order_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_order_timer();
	order_dlg->exec();
}

void main_dialog::call_carderr_reason_dlg()
{
	timer->stop();
	if ( carderr_reason_dlg )
	{
		delete carderr_reason_dlg;
		carderr_reason_dlg = NULL;
	}
	carderr_reason_dlg = new  dialog_carderr_reason;
	carderr_reason_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_carderr_reason_timer() ),carderr_reason_dlg,SLOT ( init_page_dlg() ) );
	connect ( carderr_reason_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_carderr_reason_timer();
	carderr_reason_dlg->exec();
}

void main_dialog::call_endword_dlg()
{
	timer->stop();
	if ( endword_dlg )
	{
		delete endword_dlg;
		endword_dlg = NULL;
	}
	endword_dlg = new  dialog_endword;
	endword_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_endword_timer() ),endword_dlg,SLOT ( init_page_dlg() ) );
	connect ( endword_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_endword_timer();
	endword_dlg->exec();
}

void main_dialog::call_chargerecode_dlg()
{
	timer->stop();
	if ( chargerecode_dlg )
	{
		delete chargerecode_dlg;
		chargerecode_dlg = NULL;
	}
	chargerecode_dlg = new  dialog_chargerecode;
	chargerecode_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_chargerecode_timer() ),chargerecode_dlg,SLOT ( init_page_dlg() ) );
	connect ( chargerecode_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_chargerecode_timer();
	chargerecode_dlg->exec();
}

void main_dialog::call_fail_start_reason_dlg()
{
	timer->stop();
	if ( fail_start_reason_dlg )
	{
		delete fail_start_reason_dlg;
		fail_start_reason_dlg = NULL;
	}
	fail_start_reason_dlg = new  dialog_fail_start_reason;
	fail_start_reason_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_failstart_timer() ),fail_start_reason_dlg,SLOT ( init_page_dlg() ) );
	connect ( fail_start_reason_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_failstart_timer();
	fail_start_reason_dlg->exec();
}

void main_dialog::call_local_charge_recode_dlg()
{
	timer->stop();
	if ( local_charge_recode_dlg )
	{
		delete local_charge_recode_dlg;
		local_charge_recode_dlg = NULL;
	}
	local_charge_recode_dlg = new  dialog_local_charge_recode;
	local_charge_recode_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_localcharge_timer() ),local_charge_recode_dlg,SLOT ( init_page_dlg() ) );
	connect ( local_charge_recode_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_localcharge_timer();
	local_charge_recode_dlg->exec();
}

void main_dialog::call_remote_unload_recode_dlg()
{
	timer->stop();
	if ( remote_unload_recode_dlg )
	{
		delete remote_unload_recode_dlg;
		remote_unload_recode_dlg = NULL;
	}
	remote_unload_recode_dlg = new  dialog_remote_unload_recode;
	remote_unload_recode_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_remotecharge_timer() ),remote_unload_recode_dlg,SLOT ( init_page_dlg() ) );
	connect ( remote_unload_recode_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_remotecharge_timer();
	remote_unload_recode_dlg->exec();
}

void main_dialog::call_module_monitor_dlg()
{
	timer->stop();
	if ( module_monitor_dlg )
	{
		delete module_monitor_dlg;
		module_monitor_dlg = NULL;
	}
	module_monitor_dlg = new  dialog_module_monitor;
	module_monitor_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_modulemonitor_timer() ),module_monitor_dlg,SLOT ( init_page_dlg() ) );
	connect ( module_monitor_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_modulemonitor_timer();
	module_monitor_dlg->exec();
}

void main_dialog::call_module_err_info_dlg()
{
	timer->stop();
	if ( module_err_info_dlg )
	{
		delete module_err_info_dlg;
		module_err_info_dlg = NULL;
	}
	module_err_info_dlg = new  dialog_module_err_info;
	module_err_info_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_moduleerr_timer() ),module_err_info_dlg,SLOT ( init_page_dlg() ) );
	connect ( module_err_info_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_moduleerr_timer();
	module_err_info_dlg->exec();
}

void main_dialog::call_module_output_dlg()
{
	timer->stop();
	if ( module_output_dlg )
	{
		delete module_output_dlg;
		module_output_dlg = NULL;
	}
	module_output_dlg = new  dialog_module_output;
	module_output_dlg->setWindowFlags ( Qt::FramelessWindowHint );
	connect ( this,SIGNAL ( signal_enable_moduleout_timer() ),module_output_dlg,SLOT ( init_page_dlg() ) );
	connect ( module_output_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
	emit signal_enable_moduleout_timer();
	module_output_dlg->exec();
}

//**************************************************************

void main_dialog::call_swingcard_err_dlg()
{
    timer->stop();
    if ( swingcard_err_dlg )
    {
        delete swingcard_err_dlg;
        swingcard_err_dlg = NULL;
    }
    swingcard_err_dlg = new  dialog_swingcard_err;
    swingcard_err_dlg->setWindowFlags ( Qt::FramelessWindowHint );
    connect ( this,SIGNAL ( signal_enable_swingcard_err_timer() ),swingcard_err_dlg,SLOT ( init_page_dlg() ) );
    connect ( swingcard_err_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
    emit signal_enable_swingcard_err_timer();
    swingcard_err_dlg->exec();
}

void main_dialog::call_wait_swingcard_dlg()
{
    timer->stop();
    if ( wait_swingcard_dlg )
    {
        delete wait_swingcard_dlg;
        wait_swingcard_dlg = NULL;
    }
    wait_swingcard_dlg = new  dialog_wait_swingcard;
    wait_swingcard_dlg->setWindowFlags ( Qt::FramelessWindowHint );
    connect ( this,SIGNAL ( signal_enable_wait_swingcard_timer() ),wait_swingcard_dlg,SLOT ( init_page_dlg() ) );
    connect ( wait_swingcard_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
    emit signal_enable_wait_swingcard_timer();
    wait_swingcard_dlg->exec();
}

void main_dialog::call_stoping_stub_dlg()
{
    timer->stop();
    if ( stoping_stub_dlg )
    {
        delete stoping_stub_dlg;
        stoping_stub_dlg = NULL;
    }
    stoping_stub_dlg = new  dialog_stoping_stub;
    stoping_stub_dlg->setWindowFlags ( Qt::FramelessWindowHint );
    connect ( this,SIGNAL ( signal_enable_stoping_stub_timer() ),stoping_stub_dlg,SLOT ( init_page_dlg() ) );
    connect ( stoping_stub_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
    emit signal_enable_stoping_stub_timer();
    stoping_stub_dlg->exec();
}


void main_dialog::call_waiting_payoff_dlg()
{
    timer->stop();
    if ( waiting_payoff_dlg )
    {
        delete waiting_payoff_dlg;
        waiting_payoff_dlg = NULL;
    }
    waiting_payoff_dlg = new  dialog_waiting_payoff;
    waiting_payoff_dlg->setWindowFlags ( Qt::FramelessWindowHint );
    connect ( this,SIGNAL ( signal_enable_waiting_payoff_timer() ),waiting_payoff_dlg,SLOT ( init_page_dlg() ) );
    connect ( waiting_payoff_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
    emit signal_enable_waiting_payoff_timer();
    waiting_payoff_dlg->exec();
}

void main_dialog::call_quit_success_dlg()
{
    timer->stop();
    if ( quit_success_dlg )
    {
        delete quit_success_dlg;
        quit_success_dlg = NULL;
    }
    quit_success_dlg = new  dialog_quit_success;
    quit_success_dlg->setWindowFlags ( Qt::FramelessWindowHint );
    connect ( this,SIGNAL ( signal_enable_quit_success_timer() ),quit_success_dlg,SLOT ( init_page_dlg() ) );
    connect ( quit_success_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
    emit signal_enable_quit_success_timer();
    quit_success_dlg->exec();
}

void main_dialog::call_net_err_dlg()
{
    timer->stop();
    if ( net_err_dlg )
    {
        delete net_err_dlg;
        net_err_dlg = NULL;
    }
    net_err_dlg = new  dialog_net_err;
    net_err_dlg->setWindowFlags ( Qt::FramelessWindowHint );
    connect ( this,SIGNAL ( signal_enable_net_err_timer() ),net_err_dlg,SLOT ( init_page_dlg() ) );
    connect ( net_err_dlg,SIGNAL ( signal_exit_dlg() ),this,SLOT ( start_timer() ) );
    emit signal_enable_net_err_timer();
    net_err_dlg->exec();
}

void main_dialog::on_pbtn_card_inquiry_clicked()
{
	printf ( "********** go to card_inquiry page ************\n" );
	//hmi_button_dn_num=CARD_INQUIRY_BUTTON;
	//API_Write_DB_Nbyte(PL_HMI_BUTTON_DOWN,(INT8 *)(&hmi_button_dn_num),
	//                           sizeof(hmi_button_dn_num));

	hmi_page_index = PAGEINDEX_CARDINQUERY;
	API_Write_DB_Nbyte ( PL_HMI_PAGE_INDEX, ( INT8* ) ( &hmi_page_index ),sizeof ( hmi_page_index ) );
}


void main_dialog::on_pbtn_card_charge_clicked()
{
	hmi_page_index = PAGEINDEX_SWINGCARD;
	API_Write_DB_Nbyte ( PL_HMI_PAGE_INDEX, ( INT8* ) ( &hmi_page_index ),sizeof ( hmi_page_index ) );
}

void main_dialog::on_pbtn_card_unlock_clicked()
{
	hmi_page_index = PAGEINDEX_CARDUNLOCK;
	API_Write_DB_Nbyte ( PL_HMI_PAGE_INDEX, ( INT8* ) ( &hmi_page_index ),sizeof ( hmi_page_index ) );
}

void main_dialog::on_pbtn_device_info_clicked()
{
	hmi_page_index = PAGEINDEX_DEVICEINFO;
	API_Write_DB_Nbyte ( PL_HMI_PAGE_INDEX, ( INT8* ) ( &hmi_page_index ),sizeof ( hmi_page_index ) );
}

void main_dialog::on_pbtn_charge_price_clicked()
{
	hmi_page_index = PAGEINDEX_CHARGEPRICE;
	API_Write_DB_Nbyte ( PL_HMI_PAGE_INDEX, ( INT8* ) ( &hmi_page_index ),sizeof ( hmi_page_index ) );
}

void main_dialog::on_pbtn_app_download_clicked()
{
	hmi_page_index = PAGEINDEX_APPDOWNLOAD;
	API_Write_DB_Nbyte ( PL_HMI_PAGE_INDEX, ( INT8* ) ( &hmi_page_index ),sizeof ( hmi_page_index ) );
}

void main_dialog::on_pbtn_setpara_clicked()
{
	hmi_page_index = PAGEINDEX_SETPARAPASSWD;
	API_Write_DB_Nbyte ( PL_HMI_PAGE_INDEX, ( INT8* ) ( &hmi_page_index ),sizeof ( hmi_page_index ) );
}

void main_dialog::setframe_screensr1_up()
{
	if ( click_screen_flag == 0 )
	{

		ui->frame_ss1->move ( 0,0 );
		ui->frame_ss2->move ( 0,800 );
		ui->frame_main->move ( 0,800 );

		timedelay_msec ( 5000 );
		setframe_screensr2_up();

		if ( hmi_page_index != PAGEINDEX_MAIN )
		{
			setframe_screensr_flag = 2;
			check_hmi_page_index();

			ui->frame_ss1->move ( 0,800 );
			ui->frame_ss2->move ( 0,800 );
			ui->frame_main->move ( 0,0 );
		}
	}
	else
	{

	}

}

void main_dialog::setframe_screensr2_up()
{

	if ( click_screen_flag == 0 )
	{
		ui->frame_ss1->move ( 0,800 );
		ui->frame_ss2->move ( 0,0 );
		ui->frame_main->move ( 0,800 );

		timedelay_msec ( 5000 );
		setframe_screensr1_up();

		if ( hmi_page_index != PAGEINDEX_MAIN )
		{
			setframe_screensr_flag = 2;
			check_hmi_page_index();

			ui->frame_ss1->move ( 0,800 );
			ui->frame_ss2->move ( 0,800 );
			ui->frame_main->move ( 0,0 );
		}
	}
	else
	{

	}
}

void main_dialog::on_pbtn_screensaver1_clicked()
{
	ui->frame_ss1->move ( 0,800 );
	ui->frame_ss2->move ( 0,800 );
	ui->frame_main->move ( 0,0 );

	click_screen_flag = 1;
}

void main_dialog::on_pbtn_screensaver2_clicked()
{
	ui->frame_ss1->move ( 0,800 );
	ui->frame_ss2->move ( 0,800 );
	ui->frame_main->move ( 0,0 );

	click_screen_flag= 1;
}
