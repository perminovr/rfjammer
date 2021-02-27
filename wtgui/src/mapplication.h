#ifndef MAPPLICATION_H
#define MAPPLICATION_H

#include <Wt/WApplication.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WBoxLayout.h>
#include <Wt/WButtonGroup.h>
#include <Wt/WBreak.h>
#include <Wt/WCheckBox.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDoubleSpinBox.h>
#include <Wt/WEnvironment.h>
#include <Wt/WException.h>
#include <Wt/WFileUpload.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WImage.h>
#include <Wt/WIntValidator.h>
#include <Wt/WLabel.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLink.h>
#include <Wt/WMenu.h>
#include <Wt/WMenuItem.h>
#include <Wt/WMessageBox.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPopupMenuItem.h>
#include <Wt/WProgressBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WRadioButton.h>
#include <Wt/WResource.h>
#include <Wt/WServer.h>
#include <Wt/WSlider.h>
#include <Wt/WSpinBox.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WTable.h>
#include <Wt/WTemplate.h>
#include <Wt/WTextArea.h>
#include <Wt/WText.h>
#include <Wt/WTimer.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

#include <map>

#include "wtwithqt/WQApplication.h"
#include "common.h"

// qt includes not allowed here
class CtlClient;
class QTimer;

using namespace Wt;

/*!
 * @class MApplication
 * @brief Приложение, создаваемое для каждого http-клиента
 * @details взаимодейтсвует с основной программой через @ref CtlServer
 * соединения указаны в @ref MApplication::create "wt <-> qt connections"
*/
class MApplication : public Wt::WQApplication
{
public:
	MApplication(const WEnvironment &env) : Wt::WQApplication(env, true) {}
	virtual ~MApplication() {}

	static uptr<Wt::WApplication> createApplication(const Wt::WEnvironment &env)
	{ return mkuptr<MApplication>(env); }

	virtual void create() override;
	virtual void destroy() override;

	struct WebDeviceData {
		DevStruct::Device d;
		bool connected;

		Wt::WMenuItem *mitem = nullptr;

		Wt::WSpinBox *ctl_siglvl = nullptr;
		Wt::WButtonGroup *ctl_en = nullptr;

		Wt::WText *stat_siglvl = nullptr;
		Wt::WText *stat_en = nullptr;
		Wt::WText *stat_T = nullptr;
		Wt::WText *stat_rfpwr = nullptr;
		Wt::WText *stat_rfout = nullptr;
        Wt::WText *stat_ta = nullptr;
        Wt::WText *stat_va = nullptr;
		Wt::WText *stat_it = nullptr;
		Wt::WText *stat_U = nullptr;
		Wt::WText *stat_cio = nullptr;

		WebDeviceData() {}
		WebDeviceData(const WebDeviceData &wd) { *this = wd; }
		WebDeviceData(const DevStruct::Device &d) { *this = d; }
		void operator=(const DevStruct::Device &d) { this->d = d; }
		void operator=(const WebDeviceData &wd) { this->d = wd.d; }
	};
	struct WebNetworking {
		int mode = 0;
		char ip[17] = "";
		char mask[17] = "";
		char gw[17] = "";
	};
	struct WebInfoServer {
		bool servOnVpn = false;
		uint16_t servPort = 0;
	};

private:
	std::string m_sessionId;

	CtlClient *qtcore;
    QTimer *timer;

	Wt::WMenu *dev_submenu = nullptr;
	std::map <int, WebDeviceData> dev_data;
    Wt::WLineEdit *new_dev_id = nullptr;

	Wt::WLineEdit *radio_ta = nullptr;
	Wt::WLineEdit *radio_tr = nullptr;
	Wt::WLineEdit *radio_sp = nullptr;
	Wt::WLineEdit *radio_pp = nullptr;

	Wt::WComboBox *netw_mode = nullptr;
	Wt::WLineEdit *netw_ip = nullptr;
	Wt::WLineEdit *netw_mask = nullptr;
	Wt::WLineEdit *netw_gw = nullptr;
	Wt::WButtonGroup *netw_sslot = nullptr;
	Wt::WLineEdit *netw_sport = nullptr;

	Wt::WText *time_cur = nullptr;
	Wt::WLineEdit *time_set = nullptr;

	Wt::WText *vpn_ip = nullptr;
	Wt::WButtonGroup *vpn_en = nullptr;

	uptr<Wt::WWidget> newDeviceItem(MApplication::WebDeviceData &wd);
	uptr<Wt::WWidget> createDevicesItem();
	uptr<Wt::WWidget> radioItem();
	uptr<Wt::WWidget> networkingItem();
	uptr<Wt::WWidget> timeItem();
	uptr<Wt::WWidget> createVpnItem();

	void updateDeviceItem(const MApplication::WebDeviceData &wd, int id);
	void updateRadioItem(const ServStruct::RadioParams &p);
	void updateNetworkingItem(const WebNetworking &n);
	void updateServerItem(const WebInfoServer &n);
	void updateTimeItem(const Wt::WString &time);
	void updateVpnItem(const Wt::WString &ip);

	void onNetModeChanged();
	void addDeviceItem(int id, bool con, const DevStruct::Device &d);;
};

#endif // MAPPLICATION_H
