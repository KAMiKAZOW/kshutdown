
//!!!cleanup

#include <QGroupBox>
#include <QLayout>
#include <QTimer>

#include <KComboBox>
#include <KLocale>
#include <KMenu>
#include <KMenuBar>
#include <KPushButton>
#include <KStandardAction>
#include <KStandardGuiItem>
#include <KVBox>

#include "mainwindow.h"
#include "mainwindow.moc"

MainWindow *MainWindow::m_instance = 0;

// public

MainWindow::~MainWindow() {
	kDebug() << "MainWindow::~MainWindow()" << endl;

	writeConfig();
}

// private

MainWindow::MainWindow() :
	KMainWindow(),
	m_active(false),
	m_forceQuit(false),
	m_actionHash(QHash<QString, Action*>()),
	m_triggerHash(QHash<QString, Trigger*>()),
	m_triggerTimer(new QTimer(this)),
	m_currentActionWidget(0),
	m_currentTriggerWidget(0) {

	kDebug() << "MainWindow::MainWindow()" << endl;

	setObjectName("MainWindow::MainWindow");

	// NOTE: do not change the "init" order,
	// or your computer will explode
	initWidgets();
	initActions();
	initTriggers();
	initPlugins();
	initMenuBar();

	readConfig();
}

void MainWindow::addAction(Action *action) {
	m_actions->addItem(action->icon(), action->text(), action->id());
	int index = m_actions->count() - 1;
	m_actionHash[action->id()] = action;

	kDebug() << "\tMainWindow::addAction( \"" << action->text() << "\" ) [ id=" << action->id() << ", index=" << index << " ]" << endl;
}

void MainWindow::addTrigger(Trigger *trigger) {
	m_triggers->addItem(trigger->icon(), trigger->text(), trigger->id());
	int index = m_triggers->count() - 1;
	m_triggerHash[trigger->id()] = trigger;

	kDebug() << "\tMainWindow::addTrigger( \"" << trigger->text() << "\" ) [ id=" << trigger->id() << ", index=" << index << " ]" << endl;
}

Action *MainWindow::getSelectedAction() const {
	return m_actionHash[m_actions->itemData(m_actions->currentIndex()).toString()];
}

void MainWindow::setSelectedAction(const QString &id) {
	kDebug() << "MainWindow::setSelectedAction( " << id << " )" << endl;

	onActionActivated(selectById(m_actions, id));
}

Trigger *MainWindow::getSelectedTrigger() const {
	return m_triggerHash[m_triggers->itemData(m_triggers->currentIndex()).toString()];
}

void MainWindow::setSelectedTrigger(const QString &id) {
	kDebug() << "MainWindow::setSelectedTrigger( " << id << " )" << endl;

	onTriggerActivated(selectById(m_triggers, id));
}

void MainWindow::initActions() {
	kDebug() << "MainWindow::initActions()" << endl;

	addAction(LockAction::self());
	addAction(new LogoutAction());
	addAction(new ShutDownAction());
	addAction(new RebootAction());
	addAction(new SuspendAction());
	addAction(new HibernateAction());
}

// TODO: action/trigger presets

void MainWindow::initMenuBar() {
	kDebug() << "MainWindow::initMenuBar()" << endl;

	KMenuBar *menuBar = new KMenuBar();

	KMenu *fileMenu = new KMenu(i18n("&File"));

	fileMenu->addTitle(KIcon("messagebox_warning"), i18n("No Delay"));
	QString id;
	for (int i = 0; i < m_actions->count(); ++i) {
		id = m_actions->itemData(i).toString();
		fileMenu->addAction(m_actionHash[id]);
		if ((id == "logout") || (id == "reboot"))
			fileMenu->addSeparator();
	}

	fileMenu->addSeparator();
	fileMenu->addAction(KStandardAction::quit(this, SLOT(onQuit()), this));

	menuBar->addMenu(fileMenu);

	KMenu *settingsMenu = new KMenu(i18n("&Settings"));
	settingsMenu->addTitle("SORRY, UNDER CONSTRUCTION :)");//!!!
	menuBar->addMenu(settingsMenu);

// FIXME: too many menu items
	menuBar->addMenu(helpMenu());

	setMenuBar(menuBar);
}

// TODO: plugins
void MainWindow::initPlugins() {
	kDebug() << "MainWindow::initPlugins()" << endl;
}

void MainWindow::initTriggers() {
	kDebug() << "MainWindow::initTriggers()" << endl;

	connect(m_triggerTimer, SIGNAL(timeout()), SLOT(onCheckTrigger()));

	addTrigger(new NoDelayTrigger());
	addTrigger(new TimeFromNowTrigger());
	addTrigger(new DateTimeTrigger());
	//!!!
}

void MainWindow::initWidgets() {
	KVBox *mainWidget = new KVBox();
	mainWidget->setMargin(5);
	mainWidget->setSpacing(5);

	m_actionBox = new QGroupBox(i18n("Select an &action"), mainWidget);
	m_actionBox->setLayout(new QVBoxLayout());

	KHBox *hBox = new KHBox();
	hBox->setSpacing(5);
	m_actionBox->layout()->addWidget(hBox);

	m_actions = new KComboBox(hBox);
	connect(m_actions, SIGNAL(activated(int)), SLOT(onActionActivated(int)));

	m_configureActionButton = new KPushButton(hBox);
	m_configureActionButton->setGuiItem(KStandardGuiItem::configure());
	m_configureActionButton->setToolTip(i18n("Configure selected action"));
	connect(m_configureActionButton, SIGNAL(clicked()), SLOT(onConfigureAction()));

	m_triggerBox = new QGroupBox(i18n("S&elect a time"), mainWidget);
	m_triggerBox->setLayout(new QVBoxLayout());

	m_triggers = new KComboBox();
	connect(m_triggers, SIGNAL(activated(int)), SLOT(onTriggerActivated(int)));
	m_triggerBox->layout()->addWidget(m_triggers);

	QVBoxLayout *mainWidgetLayout = static_cast<QVBoxLayout *>(mainWidget->layout());
	mainWidgetLayout->addStretch();

// TODO: align center
	m_okCancelButton = new KPushButton(mainWidget);
	m_okCancelButton->setDefault(true);
	connect(m_okCancelButton, SIGNAL(clicked()), SLOT(onOKCancel()));

	setCentralWidget(mainWidget);
}

void MainWindow::pluginConfig(const bool read) {
	KConfig *config = KGlobal::config().data();//!!!

	foreach (Action *i, m_actionHash) {
		config->setGroup("KShutdown Action " + i->id());
		if (read)
			i->readConfig(config);
		else
			i->writeConfig(config);
	}

	foreach (Trigger *i, m_triggerHash) {
		config->setGroup("KShutdown Trigger " + i->id());
		if (read)
			i->readConfig(config);
		else
			i->writeConfig(config);
	}
}

void MainWindow::readConfig() {
	kDebug() << "MainWindow::readConfig()" << endl;

	KConfig *config = KGlobal::config().data();

	pluginConfig(true); // read

	config->setGroup("General");
	setSelectedAction(config->readEntry("Selected Action"));
	setSelectedTrigger(config->readEntry("Selected Trigger"));
}

int MainWindow::selectById(KComboBox *comboBox, const QString &id) {
	int index = comboBox->findData(id);
	if (index == -1)
		index = 0;
	comboBox->setCurrentIndex(index);

	return index;
}

void MainWindow::setActive(const bool yes) {
	kDebug() << "MainWindow::setActive( " << yes << " )" << endl;

	if (m_active == yes)
		return;

	m_active = yes;

	Action *action = getSelectedAction();
	kDebug() << "\tMainWindow::getSelectedAction() == " << action->id() << endl;
	Trigger *trigger = getSelectedTrigger();
	kDebug() << "\tMainWindow::getSelectedTrigger() == " << trigger->id() << endl;

	if (m_active) {
		m_triggerTimer->start(trigger->checkTimeout());
		action->setState(Base::START);
		trigger->setState(Base::START);
	}
	else {
		m_triggerTimer->stop();
		action->setState(Base::STOP);
		trigger->setState(Base::STOP);
	}

	// clear status
	setCaption(QString::null);

	updateWidgets();
}

void MainWindow::updateWidgets() {
	kDebug() << "MainWindow::updateWidgets()" << endl;

	bool enabled = !m_active;
	m_actions->setEnabled(enabled);
	if (m_currentActionWidget)
		m_currentActionWidget->setEnabled(enabled);
	m_configureActionButton->setEnabled(enabled);
	m_triggers->setEnabled(enabled);
	if (m_currentTriggerWidget)
		m_currentTriggerWidget->setEnabled(enabled);


	Action *action = getSelectedAction();
	if (action->isEnabled()) {
		m_okCancelButton->setEnabled(true);
		m_okCancelButton->setGuiItem(
			m_active
			? KStandardGuiItem::cancel()
			: KStandardGuiItem::ok()
		);
	}
	else {
		m_okCancelButton->setEnabled(false);
		m_okCancelButton->setIcon(KIcon("messagebox_critical"));
// TODO: show solution dialog
		m_okCancelButton->setText(i18n("Action not available: %0").arg(action->originalText()));
	}


// FIXME: adjust window to its minimum/preferred size
}

void MainWindow::writeConfig() {
	kDebug() << "MainWindow::writeConfig()" << endl;

	KConfig *config = KGlobal::config().data();

	pluginConfig(false); // write

	config->setGroup("General");
	config->writeEntry("Selected Action", getSelectedAction()->id());
	config->writeEntry("Selected Trigger", getSelectedTrigger()->id());

	config->sync();
}

// private slots

void MainWindow::onActionActivated(int index) {
	kDebug() << "MainWindow::onActionActivated( " << index << " )" << endl;

	if (m_currentActionWidget) {
		m_actionBox->layout()->removeWidget(m_currentActionWidget);
		m_currentActionWidget->hide();
	}

	m_currentActionWidget = getSelectedAction()->getWidget();
	if (m_currentActionWidget) {
		m_actionBox->layout()->addWidget(m_currentActionWidget);
		m_currentActionWidget->show();
	}

	updateWidgets();
}

void MainWindow::onCheckTrigger() {
	if (!m_active) {
		kDebug() << "MainWindow::onCheckTrigger(): INTERNAL ERROR #1" << endl;

		return;
	}

	Action *action = getSelectedAction();
	Trigger *trigger = getSelectedTrigger();

	if (trigger->canActivateAction()) {
		setActive(false);
		if (action->isEnabled())
			action->activate(Action::Trigger);
	}
	// update status
	else {
		QString actionStatus = action->originalText();
		QString triggerStatus = trigger->status();
		QString title = triggerStatus;
		if (!actionStatus.isEmpty()) {
			if (!title.isEmpty())
				title += " - ";
			title += actionStatus;
		}
		setCaption(title);
	}
}

void MainWindow::onConfigureAction() {
	kDebug() << "MainWindow::onConfigureAction()" << endl;
	//!!!
}

void MainWindow::onOKCancel() {
	kDebug() << "MainWindow::onOKCancel()" << endl;

	setActive(!m_active);
}

void MainWindow::onQuit() {
	kDebug() << "MainWindow::onQuit()" << endl;

	m_forceQuit = true;//!!!not used
	close();
}

void MainWindow::onTriggerActivated(int index) {
	kDebug() << "MainWindow::onTriggerActivated( " << index << " )" << endl;

	if (m_currentTriggerWidget) {
		m_triggerBox->layout()->removeWidget(m_currentTriggerWidget);
		m_currentTriggerWidget->hide();
	}

	m_currentTriggerWidget = getSelectedTrigger()->getWidget();
	if (m_currentTriggerWidget) {
		m_triggerBox->layout()->addWidget(m_currentTriggerWidget);
		m_currentTriggerWidget->show();
	}

	updateWidgets();
}
