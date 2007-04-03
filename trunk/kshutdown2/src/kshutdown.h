
//!!!cleanup

#ifndef __KSHUTDOWN_H__
#define __KSHUTDOWN_H__

// TODO: libkshutdown

#include <QDateTime>

#include <KAction>
#include <KConfig>
#include <kdemacros.h>
#include <libworkspace/kworkspace.h>

class QDateTimeEdit;

namespace KShutdown {

class KDE_EXPORT Base {
public:
// TODO: ENABLE, DISABLE, SELECTED, UNSELECTED
	enum State { START, STOP };
	Base(const QString &id);
	virtual ~Base();
	inline QString error() const {
		return m_error;
	}
	virtual QWidget *getWidget();
	inline QString id() const {
		return m_id;
	}
	inline QString originalText() const {
		return m_originalText;
	}
	virtual void readConfig(KConfig *config);
	virtual void setState(const State state);
	inline QString status() const {
		return m_status;
	}
	virtual void writeConfig(KConfig *config);
protected:
	QString m_error;
	QString m_id;

	/**
	 * A text without "&" shortcut.
	 */
	QString m_originalText;

	QString m_status;
};

class KDE_EXPORT Action: public KAction, public Base {
	Q_OBJECT
public:
	Action(const QString &text, const QString &iconName, const QString &id);
	virtual bool onAction() = 0;
private slots:
	void slotFire();
};

class KDE_EXPORT Trigger: public QObject, public Base {
	Q_OBJECT
public:
	Trigger(const QString &text, const QString &iconName, const QString &id);
	inline KIcon icon() const {
		return m_icon;
	}
	virtual bool canActivateAction() = 0;
	inline int checkTimeout() const {
		return m_checkTimeout;
	}
	inline QString text() const {
		return m_text;
	}
protected:
	int m_checkTimeout;
private:
	KIcon m_icon;
	QString m_text;
};

class KDE_EXPORT DateTimeTriggerBase: public Trigger {
	Q_OBJECT
public:
	DateTimeTriggerBase(const QString &text, const QString &iconName, const QString &id);
	virtual bool canActivateAction();
	virtual QWidget *getWidget();
	virtual void readConfig(KConfig *config);
	virtual void writeConfig(KConfig *config);
protected:
	QDateTime m_dateTime;
	QDateTime m_endDateTime;
	QDateTimeEdit *m_edit;
private slots:
	void syncDateTime();
};

class KDE_EXPORT DateTimeTrigger: public DateTimeTriggerBase {
public:
	DateTimeTrigger();
	virtual QWidget *getWidget();
	virtual void setState(const State state);
};

class KDE_EXPORT NoDelayTrigger: public Trigger {
public:
	NoDelayTrigger();
	virtual bool canActivateAction();
};

class KDE_EXPORT TimeFromNowTrigger: public DateTimeTriggerBase {
public:
	TimeFromNowTrigger();
	virtual QWidget *getWidget();
	virtual void setState(const State state);
};

class KDE_EXPORT PowerAction: public Action {
public:
	PowerAction(const QString &text, const QString &iconName, const QString &id);
	virtual bool onAction();
protected:
	QString m_methodName;
	bool isAvailable(const QString &feature) const;
};

class KDE_EXPORT HibernateAction: public PowerAction {
public:
	HibernateAction();
};

class KDE_EXPORT SuspendAction: public PowerAction {
public:
	SuspendAction();
};

class KDE_EXPORT LockAction: public Action {
public:
	virtual bool onAction();
	inline static LockAction *self() {
		if (!m_instance)
			m_instance = new LockAction();

		return m_instance;
	}
private:
	static LockAction *m_instance;
	LockAction();
};

class KDE_EXPORT StandardKDEAction: public Action {
public:
	StandardKDEAction(const QString &text, const QString &iconName, const QString &id, const KWorkSpace::ShutdownType type);
	virtual bool onAction();
private:
	KWorkSpace::ShutdownType m_type;
};

class KDE_EXPORT LogoutAction: public StandardKDEAction {
public:
	LogoutAction();
};

class KDE_EXPORT RebootAction: public StandardKDEAction {
public:
	RebootAction();
};

class KDE_EXPORT ShutDownAction: public StandardKDEAction {
public:
	ShutDownAction();
};

} // KShutdown

#endif // __KSHUTDOWN_H__
