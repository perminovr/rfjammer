#include "mapplication.h"
#include "ctlclient.h"
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QDateTime>
#include <sys/time.h>
#include "staticmembers.h"


typedef std::vector<WString> WStringList;
#define tr(x) Wt::WString::tr(x)


static Wt::WString toWebDeviceId(int id, bool c, bool e)
{
	std::string pat = "id_00000 [X,X]"; // 7 10 12
	std::string sid = std::to_string(id);
	int sz = sid.size();
	int i = 0;
	while (sz > 0) {
		pat[7-(i++)] = sid[--sz];
	}
	if (c) pat[10] = 'C';
	pat[12] = (e)? 'E' : 'D';
	return Wt::WString(pat);
}


void MApplication::updateDeviceItem(const MApplication::WebDeviceData &wd, int id)
{
	const DevStruct::Device &d = wd.d;
	auto mitem = wd.mitem;

	mitem->setText(toWebDeviceId(id, wd.connected, d.getMM(stat, ch.devEn)));

	wd.ctl_siglvl->setValue(d.getMM(ctl, ch.signalLvl));
	wd.ctl_en->setSelectedButtonIndex( (d.getMM(ctl, ch.devEn))? 1:0 );

	wd.stat_siglvl->setText(std::to_string(scast(int, d.getMM(stat, ch.signalLvl))));
	wd.stat_en->setText( (d.getMM(stat, ch.devEn))? "true":"false" );
	wd.stat_T->setText(std::to_string(scast(int, d.getMM(stat, ch.T))));
	wd.stat_rfpwr->setText(std::to_string(scast(int, d.getMM(stat, ch.reflectedPwr))));
	wd.stat_rfout->setText(std::to_string(scast(int, d.getMM(stat, ch.outputRF))));
    wd.stat_ta->setText( (d.getMM(stat, ch.T_alarm))? "true":"false" );
    wd.stat_va->setText( (d.getMM(stat, ch.VSWR_alarm))? "true":"false" );
	wd.stat_it->setText(std::to_string(scast(int, d.getMM(stat, aux.T))));
	wd.stat_U->setText(std::to_string(scast(int, d.getMM(stat, aux.U))));
	wd.stat_cio->setText( (d.getMM(stat, caseIsOpened))? "true":"false" );
}


void MApplication::updateRadioItem(const ServStruct::RadioParams &p)
{
	radio_ta->setText(std::to_string(scast(int, p.timeoutAck)));
	radio_tr->setText(std::to_string(scast(int, p.timeoutRepeat)));
	radio_sp->setText(std::to_string(scast(int, p.slavePause)));
	radio_pp->setText(std::to_string(scast(int, p.pollPause)));
}


void MApplication::updateNetworkingItem(const WebNetworking &n)
{
	netw_mode->setCurrentIndex(n.mode);
	netw_ip->setText(n.ip);
	netw_mask->setText(n.mask);
	netw_gw->setText(n.gw);
	onNetModeChanged();
}


void MApplication::updateServerItem(const WebInfoServer &n)
{
	netw_sslot->setSelectedButtonIndex(n.servOnVpn? 0:1);
	netw_sport->setText(std::to_string(scast(int, n.servPort)));
}


void MApplication::updateTimeItem(const Wt::WString &time)
{
	time_cur->setText(time);
}


void MApplication::updateVpnItem(const Wt::WString &ip)
{
	vpn_ip->setText(ip);
}


static inline void inputErrorMessage(const Wt::WString &info = "")
{
	const WAnimation& animation = WAnimation();
	WMessageBox box(
			tr("in-err-mbox-title"),
			Wt::WString("<p>") +
				tr("in-err-mbox-info") + info
			+ Wt::WString("</p>"),
			Icon::Critical,
			Wt::StandardButton::Ok);
	box.buttonClicked().connect(&box, &WMessageBox::accept);
	box.exec(animation);
}


static inline Wt::WString getLabelWithEdit(int marginTop)
{
	return Wt::WString(
		"<div class=\"form-group row\">"
			"<label for=\"${id:edit}\" class=\"col-sm-5\" style=\"margin-top: {1}px\">${label}</label>"
			"<div class=\"col-sm-7\">"
				"${edit}"
			"</div>"
		"</div>"
	).arg(marginTop);
}


static inline Wt::WString getBorderedBox(const Wt::WString &header)
{
	return Wt::WString(
		"<fieldset class=\"example\">"
			"<legend>{1}</legend>"
			"${content}"
		"</fieldset>"
	).arg(header);
}


static uptr<Wt::WPushButton> getCustomButton(const Wt::WString &text, const WLength &maxWidth = WLength("130px"))
{
	auto btn = mkuptr<Wt::WPushButton>(text);
	btn->setMaximumSize(maxWidth, "34px");
	return btn;
}


static inline uptr<Wt::WPushButton> getApplyButton()
{
	return getCustomButton(tr("btn-apply"));
}


static uptr<Wt::WWidget> labelAndWidget(uptr <Wt::WWidget> widget, const Wt::WString &labelText, int marginTop = 5)
{
	auto group = mkuptr<Wt::WTemplate>(getLabelWithEdit(marginTop));
	group->bindString("label", labelText);
	group->bindWidget("edit", mv(widget));
	group->setWidth("350px");
	return group;
}


static uptr<Wt::WWidget> lineEditDigital(Wt::WLineEdit **psafe, const Wt::WString &text, const Wt::WString &toolTip, int minimum, int maximum)
{
	auto edit = mkuptr<Wt::WLineEdit>();
	edit->setValidator(mkshptr<Wt::WIntValidator>(minimum, maximum));
	edit->setToolTip(toolTip);
	*psafe = edit.get();
	return labelAndWidget(mv(edit), text);
}


static uptr<Wt::WWidget> lineEditIp(Wt::WLineEdit **psafe, const Wt::WString &text, const Wt::WString &toolTip)
{
	auto edit = mkuptr<Wt::WLineEdit>();
	edit->setToolTip(toolTip);
	edit->setTextSize(15);
	edit->setInputMask("009.009.009.009;_");
	*psafe = edit.get();
	return labelAndWidget(mv(edit), text);
}


static uptr<Wt::WWidget> lineEditDate(Wt::WLineEdit **psafe, const Wt::WString &text, const Wt::WString &toolTip)
{
	auto edit = mkuptr<Wt::WLineEdit>();
	edit->setToolTip(toolTip);
	edit->setTextSize(16);
	edit->setInputMask("99:99 99-99-9999;_");
	*psafe = edit.get();
	return labelAndWidget(mv(edit), text);
}


static uptr<Wt::WWidget> lineComboBox(Wt::WComboBox **psafe, const Wt::WString &text, const Wt::WString &toolTip, const WStringList &model)
{
	auto cbox = mkuptr<Wt::WComboBox>();
	cbox->setToolTip(toolTip);
	for (auto &x : model)
		cbox->addItem(x);
	*psafe = cbox.get();
	return labelAndWidget(mv(cbox), text);
}


uptr<Wt::WWidget> MApplication::newDeviceItem(MApplication::WebDeviceData &wd)
{
	auto container = mkuptr<Wt::WContainerWidget>();
	container->setOverflow(Wt::Overflow::Auto);

	auto ctl = mkuptr<Wt::WTemplate>(getBorderedBox(tr("menu-devs-boxh-ctl")));
	auto stat = mkuptr<Wt::WTemplate>(getBorderedBox(tr("menu-devs-boxh-stat")));

	auto ctlContent = mkuptr<Wt::WContainerWidget>();
	{
		auto sigLvl_cont = mkuptr<Wt::WContainerWidget>();
		{
			auto sb = mkuptr<Wt::WSpinBox>();
			wd.ctl_siglvl = sb.get();
			sb->setRange(0, CONFIG_SIGNAL_LVL_VALUE_MAX);
			sb->setSingleStep(1);
			auto layout = sigLvl_cont->setLayout(mkuptr<WVBoxLayout>());
			layout->addWidget(mv(sb));
		}
		sigLvl_cont->setMargin(-20, Wt::Side::Bottom);
		sigLvl_cont->setMargin(-9, Wt::Side::Top);
		sigLvl_cont->setMargin(-10, Wt::Side::Left);
		auto sigLvl = labelAndWidget(mv(sigLvl_cont), tr("menu-devs-ctl-siglvl"));

		auto en_cont = mkuptr<Wt::WContainerWidget>();
		{
			auto group = mkshptr<Wt::WButtonGroup>();
			wd.ctl_en = group.get();
			group->addButton(en_cont->addNew<Wt::WRadioButton>(tr("radiobtn-en")));
			group->addButton(en_cont->addNew<Wt::WRadioButton>(tr("radiobtn-dis")));
			group->setSelectedButtonIndex(0);
		}
		en_cont->setMargin(5, Wt::Side::Top);
		auto en = labelAndWidget(mv(en_cont), tr("menu-devs-ctl-state"));

		// device ctl apply button
		auto btnApply = getApplyButton();
		btnApply->clicked().connect(this, [this, &wd](){
			int signalLvl = wd.ctl_siglvl->value();
			bool devEn = (wd.ctl_en->selectedButtonIndex() == 0);
			if (signalLvl >= 0 && signalLvl < 256) {
				wd.d.getMM(ctl, id) = wd.d.getMM(stat, id);
				wd.d.getMM(ctl, ch.signalLvl) = scast(uint8_t, signalLvl);
				wd.d.getMM(ctl, ch.devEn) = devEn;
				prgDebug() << "------ MApplication" << "qtcore->setDeviceCtl called:" << wd.d.getM(ctl).m.id;
				this->qtcore->setDeviceCtl(wd.d.getM(ctl));
			} else {
				wd.ctl_siglvl->setValue(wd.d.getMM(ctl, ch.signalLvl));
				wd.ctl_en->setSelectedButtonIndex(wd.d.getMM(ctl, ch.devEn));
				inputErrorMessage();
			}
		});

		auto layout = ctlContent->setLayout(mkuptr<WVBoxLayout>());
		layout->addWidget(mv(sigLvl));
		layout->addWidget(mv(en));
		layout->addWidget(mv(btnApply));
	}
	ctl->bindWidget("content", mv(ctlContent));

	auto statContent = mkuptr<Wt::WContainerWidget>();
	{
		auto table = mkuptr<Wt::WTable>();
		table->setHeaderCount(1);
		table->addStyleClass("table table-hover table-condensed form-inline");
		int it = 0;
		// header
		table->elementAt(it, 0)->addNew<WText>(tr("menu-devs-stat-h-prm"));
		table->elementAt(it, 1)->addNew<WText>(tr("menu-devs-stat-h-val"));
		table->elementAt(it, 2)->addNew<WText>(tr("menu-devs-stat-h-desc"));
		table->elementAt(it, 0)->setWidth(200);
		table->elementAt(it, 1)->setWidth(200);
		it++;

		// content
		#define addDevStatParam(prm) {\
			table->elementAt(it, 0)->addNew<WText>(tr("menu-devs-stat-" #prm)); \
			wd.stat_##prm = table->elementAt(it, 1)->addNew<WText>(""); \
			table->elementAt(it, 2)->addNew<WText>(tr("menu-devs-stat-" #prm "-desc")); \
			it++;\
		}
		addDevStatParam(siglvl);
		addDevStatParam(en);
		addDevStatParam(T);
		addDevStatParam(rfpwr);
		addDevStatParam(rfout);
        addDevStatParam(ta);
        addDevStatParam(va);
		addDevStatParam(it);
		addDevStatParam(U);
		addDevStatParam(cio);

		statContent->addWidget(mv(table));
	}
	stat->bindWidget("content", mv(statContent));

	container->addWidget(mv(ctl));
	container->addWidget(mv(stat));

	return container;
}


void MApplication::addDeviceItem(int id, bool con, const DevStruct::Device &d)
{
	dev_data.emplace(id, WebDeviceData(d));
	WebDeviceData &wd = dev_data[id];
	prgDebug() << "----- addDeviceItem" << wd.d.m.ctl.m.id;
	wd.connected = con;
	wd.mitem = dev_submenu->addItem("", newDeviceItem(wd));
	updateDeviceItem(wd, id);
}


uptr<Wt::WWidget> MApplication::createDevicesItem()
{
	auto container = mkuptr<Wt::WContainerWidget>();

	auto contentsStack = mkuptr<Wt::WStackedWidget>();
    contentsStack->addStyleClass("contents");

	auto submenu = mkuptr<Wt::WMenu>(contentsStack.get());
	dev_submenu = submenu.get();
	submenu->addStyleClass("nav-pills nav-stacked submenu");
    submenu->setWidth(150);

    auto btn = getCustomButton(tr("menu-devs-btn-add"));
    btn->clicked().connect(this, [this](){
        bool ok;
        uint id = toQString(this->new_dev_id->text()).toUInt(&ok, 16);
        if (ok)
            this->qtcore->addDevice((quint16)id);
    });

    auto devId = mkuptr<Wt::WLineEdit>();
    devId->setToolTip(tr("menu-devs-edit-tooltip"));
    devId->setMaximumSize(btn->maximumWidth().value()-20, btn->maximumHeight());
    devId->setText("0x0000");
    this->new_dev_id = devId.get();

    auto container2 = mkuptr<Wt::WContainerWidget>();
    container2->addStyleClass("submenucontainer");
    auto layout2 = container2->setLayout(mkuptr<WVBoxLayout>());
    layout2->setContentsMargins(0, 10, 0, 10);
    layout2->addWidget(mv(submenu), 1);
    layout2->addWidget(mv(devId), 0, Wt::AlignmentFlag::Center);
    layout2->addWidget(mv(btn), 0, Wt::AlignmentFlag::Center);

	auto layout = container->setLayout(mkuptr<WHBoxLayout>());
    layout->addWidget(mv(container2));
	layout->addWidget(mv(contentsStack),1);

	return container;
}


uptr<Wt::WWidget> MApplication::radioItem()
{
	auto container = mkuptr<Wt::WContainerWidget>();
	container->setOverflow(Wt::Overflow::Auto);
	container->setMaximumSize(container->maximumWidth(), 450);

	auto ta = lineEditDigital(&radio_ta,
			tr("menu-radio-ta"), tr("menu-radio-ta-tooltip"), 0, 65535);
	auto tr = lineEditDigital(&radio_tr,
			tr("menu-radio-tr"), tr("menu-radio-tr-tooltip"), 0, 65535);
	auto sp = lineEditDigital(&radio_sp,
			tr("menu-radio-sp"), tr("menu-radio-sp-tooltip"), 0, 65535);
	auto pp = lineEditDigital(&radio_pp,
			tr("menu-radio-pp"), tr("menu-radio-pp-tooltip"), 0, 65535);

	// radio params apply button
	auto btn = getApplyButton();
	btn->clicked().connect(this, [this](){
		int timeoutAck = toQString(this->radio_ta->text()).toInt();
		int timeoutRepeat = toQString(this->radio_tr->text()).toInt();
		int slavePause = toQString(this->radio_sp->text()).toInt();
		int pollPause = toQString(this->radio_pp->text()).toInt();
		if  (timeoutAck >= 0 && timeoutRepeat >= 0
			 && slavePause >= 0 && pollPause >= 0)
		{
			ServStruct::RadioParams s;
			s.timeoutAck = timeoutAck;
			s.timeoutRepeat = timeoutRepeat;
			s.slavePause = slavePause;
			s.pollPause = pollPause;
			prgDebug() << "------ MApplication" << "qtcore->setRadioPrm called:";
			this->qtcore->setRadioPrm(s);
		} else {
			inputErrorMessage();
		}
	});

	auto layout = container->setLayout(mkuptr<WVBoxLayout>());
	layout->setContentsMargins(0, 10, 0, 10);
	layout->addWidget(mv(ta), 0, Wt::AlignmentFlag::Center);
	layout->addWidget(mv(tr), 0, Wt::AlignmentFlag::Center);
	layout->addWidget(mv(sp), 0, Wt::AlignmentFlag::Center);
	layout->addWidget(mv(pp), 0, Wt::AlignmentFlag::Center);
	layout->addWidget(mv(btn), 0, Wt::AlignmentFlag::Center);

	return container;
}


void MApplication::onNetModeChanged()
{
	if (netw_mode->currentIndex() == 0) {
		netw_ip->setDisabled(false);
		netw_mask->setDisabled(false);
		netw_gw->setDisabled(false);
	} else {
		netw_ip->setDisabled(true);
		netw_mask->setDisabled(true);
		netw_gw->setDisabled(true);
	}
}


uptr<Wt::WWidget> MApplication::networkingItem()
{
	auto container = mkuptr<Wt::WContainerWidget>();
	container->setOverflow(Wt::Overflow::Auto);
	container->setMaximumSize(container->maximumWidth(), 450);

	auto mode = lineComboBox(&netw_mode,
			tr("menu-netw-mode"), tr("menu-netw-mode-tooltip"), {"Static", "Dhcp"});
	netw_mode->changed().connect(this, &MApplication::onNetModeChanged);
	auto ip = lineEditIp(&netw_ip,
			tr("menu-netw-static-ip"), tr("menu-netw-static-ip-tooltip"));
	auto mask = lineEditIp(&netw_mask,
			tr("menu-netw-mask"), tr("menu-netw-mask-tooltip"));
	auto gw = lineEditIp(&netw_gw,
			tr("menu-netw-gw"), tr("menu-netw-gw-tooltip"));
	auto slot_cont = mkuptr<Wt::WContainerWidget>();
	{
		slot_cont->setToolTip(tr("menu-netw-sslot-tooltip"));
		auto group = mkshptr<Wt::WButtonGroup>();
		netw_sslot = group.get();
		group->addButton(slot_cont->addNew<Wt::WRadioButton>(tr("menu-netw-sslot-vpn")));
		group->addButton(slot_cont->addNew<Wt::WRadioButton>(tr("menu-netw-sslot-dev")));
		group->setSelectedButtonIndex(0);
	}
	slot_cont->setMargin(5, Wt::Side::Top);
	auto sslot = labelAndWidget(mv(slot_cont), tr("menu-netw-sslot-label"));
	auto sport = lineEditDigital(&netw_sport,
			tr("menu-netw-sport"), tr("menu-netw-sport-tooltip"), 0, 65535);

	// networking apply button
	auto btn = getApplyButton();
	btn->clicked().connect(this, [this](){
		QwFrameHandler::DeviceNetParams np;
		QwFrameHandler::ServerAddr sa;
		np.ipMode = this->netw_mode->currentIndex();
		np.localAddr.setIp(QHostAddress(toQString(this->netw_ip->text())));
		np.localAddr.setNetmask(QHostAddress(toQString(this->netw_mask->text())));
		np.gw = QHostAddress(toQString(this->netw_gw->text()));
		sa.slot = scast(QwFrameHandler::ServerAddr::Slot, this->netw_sslot->selectedButtonIndex());
		sa.port = toQString(this->netw_sport->text()).toInt();
		if (np.localAddr.ip() != QHostAddress::Null
			&& np.localAddr.netmask() != QHostAddress::Null
			&& np.gw != QHostAddress::Null
			&& sa.port > 0 && sa.port < 65536
		) {
			prgDebug() << "------ MApplication" << "qtcore->setNetParam called:";
			this->qtcore->setNetParam(np);
			prgDebug() << "------ MApplication" << "qtcore->setServerAddr called:";
			this->qtcore->setServerAddr(sa);
		} else {
			inputErrorMessage();
		}
	});

	auto layout = container->setLayout(mkuptr<WVBoxLayout>());
	layout->setContentsMargins(0, 10, 0, 10);
	layout->addWidget(mv(mode), 0, Wt::AlignmentFlag::Center);
	layout->addWidget(mv(ip), 0, Wt::AlignmentFlag::Center);
	layout->addWidget(mv(mask), 0, Wt::AlignmentFlag::Center);
	layout->addWidget(mv(gw), 0, Wt::AlignmentFlag::Center);
	layout->addWidget(mv(sslot), 0, Wt::AlignmentFlag::Center);
	layout->addWidget(mv(sport), 0, Wt::AlignmentFlag::Center);
	layout->addWidget(mv(btn), 0, Wt::AlignmentFlag::Center);

	return container;
}


uptr<Wt::WWidget> MApplication::timeItem()
{
	auto container = mkuptr<Wt::WContainerWidget>();
	container->setOverflow(Wt::Overflow::Auto);
    container->setMaximumSize(container->maximumWidth(), 200);
	
	auto tc = mkuptr<Wt::WText>();
	time_cur = tc.get();
	auto tcur = labelAndWidget(mv(tc), tr("menu-time-cur"), 1);
	auto tset = lineEditDate(&time_set,
			tr("menu-time-set"), tr("menu-time-set-tooltip"));
	time_set->setText("00:00 01-01-2021");
	auto btn = getApplyButton();
	btn->clicked().connect(this, [this](){
		auto dt = QDateTime::fromString(toQString(time_set->text()), "hh:mm dd-MM-yyyy");
		if (dt.isValid())
		{
            auto d = dt.toMSecsSinceEpoch()/1000;
            struct timeval tv = { .tv_sec = scast(time_t, d), .tv_usec = 0 };
			settimeofday(&tv, 0);
			system("hwclock --systohc");
		} else {
			inputErrorMessage();
		}
    });

	auto layout = container->setLayout(mkuptr<WVBoxLayout>());
	layout->setContentsMargins(0, 10, 0, 10);
	layout->addWidget(mv(tcur), 0, Wt::AlignmentFlag::Center | Wt::AlignmentFlag::Middle);
	layout->addWidget(mv(tset), 0, Wt::AlignmentFlag::Center | Wt::AlignmentFlag::Middle);
	layout->addWidget(mv(btn), 0, Wt::AlignmentFlag::Center);

	return container;
}


/*!
 * @class OvpnExample
 * @brief Internal simple class
*/
class OvpnExample : public Wt::WResource
{
public:
	OvpnExample() : Wt::WResource() {
		suggestFileName(CONFIG_VPN_EXAMPLE_FILE_NAME);
	}
	~OvpnExample() { beingDeleted(); }
	virtual void handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response) override {
		(void)request;
		response.setMimeType("plain/text");
		QFile f(QStringLiteral(CONFIG_VPN_EXAMPLE_FILE_PATH));
		if (f.open(QFile::ReadOnly)) {
			auto data = f.readAll();
			response.out() << QString::fromUtf8(data).toStdWString() << std::endl;
		}
	}
};


uptr<Wt::WWidget> MApplication::createVpnItem()
{
	auto container = mkuptr<Wt::WContainerWidget>();
	container->setOverflow(Wt::Overflow::Auto);
	container->setMaximumSize(container->maximumWidth(), 450);

	auto logo = mkuptr<Wt::WImage>("pics/ovpn-logo3.png");
	auto resource = mkshptr<OvpnExample>();
	auto link = Wt::WLink(resource);
	link.setTarget(Wt::LinkTarget::NewWindow);
	auto anchor = mkuptr<Wt::WAnchor>(link, "");
	anchor->setImage(mv(logo));

	auto label = mkuptr<Wt::WLabel>(tr("menu-vpn-upload"));
	label->setStyleClass("h4");

	auto fu = mkuptr<Wt::WFileUpload>();
	auto pfu = fu.get();
	fu->setFileTextSize(100);
	fu->setProgressBar(mkuptr<Wt::WProgressBar>());
	fu->setToolTip(tr("menu-vpn-upload-tooltip"));

	fu->changed().connect(this, [this, pfu](){
		(void)this;
		pfu->upload();
	});

	// vpn config upload widget
	fu->uploaded().connect(this, [this, pfu](){
		auto files = pfu->uploadedFiles();
		if (files.size() > 0) {
			auto path = files[0].spoolFileName(); // /tmp/tmpfname
			files[0].stealSpoolFile(); // do not delete file from here
			prgDebug() << "------ MApplication" << "qtcore->setVpnFileName called:";
			this->qtcore->setVpnFileName(toQString(path));
		}
	});

	// vpn enable/disable btn group
	auto en_cont = mkuptr<Wt::WContainerWidget>();
	{
		auto group = mkshptr<Wt::WButtonGroup>();
		vpn_en = group.get();
		group->addButton(en_cont->addNew<Wt::WRadioButton>(tr("radiobtn-en")));
		group->addButton(en_cont->addNew<Wt::WRadioButton>(tr("radiobtn-dis")));
		group->setSelectedButtonIndex(0);
		auto pgroup = group.get();
		group->checkedChanged().connect(this, [this, pgroup](){
			vpn_ip->setText("");
			prgDebug() << "------ MApplication" << "qtcore->setVpnEnabled called:";
			this->qtcore->setVpnEnabled(pgroup->selectedButtonIndex() == 0);
		});
	}
	auto en = labelAndWidget(mv(en_cont), tr("menu-devs-ctl-state"), 1);

	auto ip = mkuptr<Wt::WText>();
	vpn_ip = ip.get();
	auto current_ip = labelAndWidget(mv(ip), tr("menu-vpn-ip"), 1);

	auto container2 = mkuptr<Wt::WContainerWidget>();
	auto layout2 = container2->setLayout(mkuptr<WVBoxLayout>());
	label->setMargin(-5, Wt::Side::Bottom);
	en->setMargin(50, Wt::Side::Top);
	current_ip->setMargin(-10, Wt::Side::Top);
	layout2->addWidget(mv(label));
	layout2->addWidget(mv(fu));
	layout2->addWidget(mv(en));
	layout2->addWidget(mv(current_ip));

	auto layout = container->setLayout(mkuptr<WVBoxLayout>());
	layout->setContentsMargins(0, 10, 0, 10);
	container2->setMargin(50, Wt::Side::Top);
	layout->addWidget(mv(anchor), 0, Wt::AlignmentFlag::Center);
	layout->addWidget(mv(container2), 0, Wt::AlignmentFlag::Center);

	return container;
}


void MApplication::create()
{
	m_sessionId = Wt::WApplication::instance()->sessionId();

	std::cout << appRoot() + "text" << std::endl;
	messageResourceBundle().use(appRoot() + "text");

	setTitle(tr("main-title"));
	enableUpdates(true);

	{
		auto bootstrapTheme = mkshptr<WBootstrapTheme>();
		bootstrapTheme->setVersion(BootstrapVersion::v3);
		bootstrapTheme->setResponsive(true);
		setTheme(bootstrapTheme);
		useStyleSheet("resources/themes/bootstrap/3/bootstrap-theme.min.css");
	}

	auto navigation = mkuptr<Wt::WNavigationBar>();
	{
		navigation->setTitle(tr("main-title"), "");
		navigation->setResponsive(true);
	}

	auto contentsStack = mkuptr<Wt::WStackedWidget>();
	{
		Wt::WAnimation animation(Wt::AnimationEffect::Fade, Wt::TimingFunction::Linear, 50);
		contentsStack->setTransitionAnimation(animation, true);
		contentsStack->addStyleClass("contents");
		contentsStack->addStyleClass("scale800");
	}

	auto navmenu = mkuptr<Wt::WMenu>(contentsStack.get());
	{
		navmenu->addItem(tr("nav-devices"), createDevicesItem());
		navmenu->addItem(tr("nav-radio"), radioItem());
		navmenu->addItem(tr("nav-netw"), networkingItem());
		navmenu->addItem(tr("nav-time"), timeItem());
		navmenu->addItem(tr("nav-vpn"), createVpnItem());
	}
	navigation->addMenu(mv(navmenu));

	auto layout = root()->setLayout(mkuptr<WVBoxLayout>());
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(mv(navigation));
	layout->addWidget(mv(contentsStack),1);

	useStyleSheet("style/everywidget.css");

	// qt

	qtcore = new CtlClient();
    timer = new QTimer();
    timer->setInterval(1000);
    QObject::connect(timer, &QTimer::timeout, [this](){
        Wt::WServer::instance()->post(m_sessionId, [this](){
            auto dt = QDateTime::currentDateTime();
            auto d = dt.toString("hh:mm:ss dd-MM-yyyy");
            this->updateTimeItem( toWString(d) );
            WApplication::instance()->triggerUpdate();
        });
    });
    timer->start();

	// wt <-> qt connections

	// see other connections by searching "qtcore" outhside this constructor

	QObject::connect(qtcore, &CtlClient::allDataChanged, [this](const QwFrameHandler::AllData &d){
		prgDebug() << "------ MApplication" << "CtlClient::allDataChanged signal:";
		Wt::WServer::instance()->post(m_sessionId, [this, d](){
			const auto &np = d.net;
			const auto &sa = d.servAddr;
			{
				WebNetworking s;
				s.mode = np.ipMode;
				strncpy(s.ip, np.localAddr.ip().toString().toLocal8Bit(), sizeof(s.ip));
				strncpy(s.mask, np.localAddr.netmask().toString().toLocal8Bit(), sizeof(s.mask));
				strncpy(s.gw, np.gw.toString().toLocal8Bit(), sizeof(s.gw));
				this->updateNetworkingItem(s);
			}
			{
				WebInfoServer s;
				s.servPort = sa.port;
				s.servOnVpn = (sa.slot == QwFrameHandler::ServerAddr::Slot::Vpn);
				this->updateServerItem(s);
			}
			this->updateRadioItem(d.rp);
			this->updateVpnItem( toWString(d.vpnAddr.toString()) );
			vpn_en->setSelectedButtonIndex(d.vpnEnabled? 0:1);
			WApplication::instance()->triggerUpdate();
		});
	});
	QObject::connect(qtcore, &CtlClient::deviceStatChanged, [this](bool valid, const DevStruct::DeviceStat &s){
		// prgDebug() << "------ MApplication" << "CtlClient::deviceStatChanged signal:" << s.m.id;
		Wt::WServer::instance()->post(m_sessionId, [this, valid, s](){
			int id = s.getM(id);
			auto it = this->dev_data.find(id);
			if (it == this->dev_data.end()) {
				DevStruct::Device d;
				d.getM(stat) = s;
				d.getMM(ctl, ch.devEn) = s.getM(ch.devEn);
				d.getMM(ctl, ch.signalLvl) = s.getM(ch.signalLvl);
				this->addDeviceItem(id, valid, d);
			} else {
				WebDeviceData &wd = it->second;
				wd.d.getM(stat) = s;
				wd.connected = valid;
				this->updateDeviceItem(wd, id);
			}
			WApplication::instance()->triggerUpdate();
		});
	});
	QObject::connect(qtcore, &CtlClient::vpnAddrChanged, [this](const QHostAddress &d){
		prgDebug() << "------ MApplication" << "CtlClient::vpnAddrChanged signal:";
		Wt::WServer::instance()->post(m_sessionId, [this, d](){
			WString ret = (d != QHostAddress::Null)? toWString(d.toString()) : "";
			this->updateVpnItem( ret );
			WApplication::instance()->triggerUpdate();
		});
	});
}


void MApplication::destroy()
{
    delete timer;
	delete qtcore;
}
