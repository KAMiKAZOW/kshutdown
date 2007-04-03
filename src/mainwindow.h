
//!!!cleanup

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

// TODO: use KDialog instead?
#include <KMainWindow>

#include "kshutdown.h"

class QGroupBox;

class KComboBox;
class KPushButton;

using namespace KShutdown;

class MainWindow: public KMainWindow {
	Q_OBJECT
public:
	virtual ~MainWindow();
	inline static MainWindow *self() {
		if (!m_instance)
			m_instance = new MainWindow();

		return m_instance;
	}
private:
	bool m_active;
	bool m_forceQuit;
	KComboBox *m_actions;
	KComboBox *m_triggers;
	KPushButton *m_configureActionButton;
	KPushButton *m_okCancelButton;
	static MainWindow *m_instance;
	QGroupBox *m_actionBox;
	QGroupBox *m_triggerBox;
	QHash<QString, Action*> m_actionHash;
	QHash<QString, Trigger*> m_triggerHash;
	QTimer *m_triggerTimer;
	QWidget *m_currentActionWidget;
	QWidget *m_currentTriggerWidget;
	MainWindow();
	void addAction(Action *action);
	void addTrigger(Trigger *trigger);
	Action *getSelectedAction() const;
	void setSelectedAction(const QString &id);
	Trigger *getSelectedTrigger() const;
	void setSelectedTrigger(const QString &id);
	void initActions();
	void initMenuBar();
	void initPlugins();
	void initTriggers();
	void initWidgets();
	void pluginConfig(const bool read);
	void readConfig();
	int selectById(KComboBox *comboBox, const QString &id);
	void setActive(const bool yes);
	void updateWidgets();
	void writeConfig();
private slots:
	void onActionActivated(int index);
	void onCheckTrigger();
	void onConfigureAction();
	void onOKCancel();
	void onQuit();
	void onTriggerActivated(int index);
};

#endif // __MAINWINDOW_H__
