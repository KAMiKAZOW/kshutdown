// password.cpp - A basic password protection
// Copyright (C) 2011  Konrad Twardowski
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include "config.h"
#include "infowidget.h"
#include "mainwindow.h"
#include "password.h"

#ifdef KS_NATIVE_KDE
	#include <KPasswordDialog>
#endif // KS_NATIVE_KDE

#ifdef KS_PURE_QT
	#include <QInputDialog>
	#include <QPointer>
#endif // KS_PURE_QT

#include <QCheckBox>
#include <QCryptographicHash>
#include <QFormLayout>
#include <QLabel>

// PasswordDialog

// public:

PasswordDialog::PasswordDialog(QWidget *parent) :
	UDialog(parent, i18n("Enter New Password"), false) {
	U_DEBUG << "PasswordDialog::PasswordDialog()" U_END;

	QVBoxLayout *mainLayout = this->mainLayout();

	// form

	QFormLayout *formLayout = new QFormLayout();
	mainLayout->addLayout(formLayout);

	m_password = new U_LINE_EDIT();
	m_password->setEchoMode(U_LINE_EDIT::Password);
	connect(
		m_password, SIGNAL(textEdited(const QString &)),
		SLOT(onPasswordChange(const QString &))
	);
	formLayout->addRow(i18n("Password:"), m_password);
	
	m_confirmPassword = new U_LINE_EDIT();
	m_confirmPassword->setEchoMode(U_LINE_EDIT::Password);
	connect(
		m_confirmPassword, SIGNAL(textEdited(const QString &)),
		SLOT(onConfirmPasswordChange(const QString &))
	);
	formLayout->addRow(i18n("Confirm Password:"), m_confirmPassword);
		
	// status/hint
		
	m_status = new InfoWidget(this);
	m_status->setText(
		"<qt>" +
		i18n("The password will be saved as SHA-1 hash.") + "<br>" +
		i18n("Short password can be easily cracked.") +
		"</qt>",
		InfoWidget::WarningType
	);
	mainLayout->addSpacing(10);
	mainLayout->addWidget(m_status);
	
	addButtonBox();

	m_password->setFocus();
}

PasswordDialog::~PasswordDialog() {
	U_DEBUG << "PasswordDialog::~PasswordDialog()" U_END;
}

void PasswordDialog::apply() {
	Config *config = Config::user();
	config->beginGroup("Password Protection");
	config->write("Hash", toHash(m_password->text()));
	config->endGroup();
	config->sync();
}

bool PasswordDialog::authorize(QWidget *parent, const QString &caption, const QString &userAction) {
	if (!Config::readBool("Password Protection", userAction, false))
		return true;

	Config *config = Config::user();
	config->beginGroup("Password Protection");
	QString hash = config->read("Hash", "").toString();
	config->endGroup();
	
	if (hash.isEmpty())
		return true;

	QString password = QString::null;
	QString prompt = i18n("Enter password to perform action: %0").arg(caption);

	#ifdef KS_NATIVE_KDE
	QPointer<KPasswordDialog> dialog = new KPasswordDialog(parent);
	dialog->setPixmap(U_ICON("kshutdown").pixmap(48, 48));
	dialog->setPrompt(prompt);
	bool ok = dialog->exec();
	if (ok)
		password = dialog->password();
	delete dialog;
	
	if (!ok)
		return false;
	#else
	bool ok;
	password = QInputDialog::getText( // krazy:exclude=qclasses
		parent,
		"KShutdown", // title
		prompt,
		QLineEdit::Password, // krazy:exclude=qclasses
		"",
		&ok
	);

	if (!ok)
		return false;
	#endif // KS_NATIVE_KDE

	if (hash != toHash(password)) {
		U_ERROR_MESSAGE(parent, i18n("Invalid password"));
		
		return false;
	}
	
	return true;
}

bool PasswordDialog::authorizeSettings(QWidget *parent) {
	return PasswordDialog::authorize(parent, i18n("Preferences"), "action/settings");
}

QString PasswordDialog::toHash(const QString &password) {
	if (password.isEmpty())
		return "";
	
	QByteArray hash = QCryptographicHash::hash(("kshutdown-" + password).toUtf8(), QCryptographicHash::Sha1);
	
	return QString(hash.toHex());
}

// private:

void PasswordDialog::updateStatus() {
	int minLength = 6;
	bool ok = m_password->text().length() >= minLength;
	if (!ok) {
		m_status->setText(i18n("Password is too short (need %1 characters or more)").arg(minLength), InfoWidget::ErrorType);
	}
	else {
		ok = (m_password->text() == m_confirmPassword->text());
		if (!ok) {
			m_status->setText(i18n("Confirmation password is different"), InfoWidget::ErrorType);
		}
		else {
			if (m_password->text() == "123456")
				m_status->setText(":-(", InfoWidget::WarningType);
			else if (m_password->text() == "dupa.8")
				m_status->setText("O_o", InfoWidget::InfoType);
			else
				m_status->setText(QString::null, InfoWidget::ErrorType);
		}
	}

	acceptButton()->setEnabled(ok);
	resize(sizeHint());
}

// private slots:

void PasswordDialog::onConfirmPasswordChange(const QString &text) {
	Q_UNUSED(text)

	updateStatus();
}

void PasswordDialog::onPasswordChange(const QString &text) {
	Q_UNUSED(text)

	updateStatus();
}

// PasswordPreferences

// public:

PasswordPreferences::PasswordPreferences(QWidget *parent) :
	QWidget(parent),
	m_configKeyRole(Qt::UserRole)
{
	U_DEBUG << "PasswordPreferences::PasswordPreferences()" U_END;

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(10);
	mainLayout->setSpacing(10);

	m_enablePassword = new QCheckBox(i18n("Enable Password Protection"));
	Config *config = Config::user();
	config->beginGroup("Password Protection");
	m_enablePassword->setChecked(
		config->read("Hash", "").toString().isEmpty()
		? Qt::Unchecked
		: Qt::Checked
	);
	config->endGroup();
	
	connect(
		m_enablePassword, SIGNAL(stateChanged(int)),
		SLOT(onEnablePassword(int))
	);
	mainLayout->addWidget(m_enablePassword);
	
	QLabel *userActionListLabel = new QLabel(i18n("Password Protected Actions:"));
	mainLayout->addWidget(userActionListLabel);
	
	m_userActionList = new U_LIST_WIDGET();
	m_userActionList->setAlternatingRowColors(true);
	
	addItem("action/settings", i18n("Settings (all)"), U_ICON("configure"));
	
	foreach (const Action *action, MainWindow::self()->actionHash().values()) {
		addItem(
			"kshutdown/action/" + action->id(),
			action->originalText(),
			action->icon()
		);
	}

	addItem("kshutdown/action/cancel", i18n("Cancel"), U_ICON("dialog-cancel"));
	addItem("action/file_quit", i18n("Quit"), U_ICON("application-exit"));

	userActionListLabel->setBuddy(m_userActionList);
	mainLayout->addWidget(m_userActionList);

	InfoWidget *kioskInfo = new InfoWidget(this);
	kioskInfo->setText("<qt>" + i18n("See Also: %0").arg("<a href=\"http://sourceforge.net/p/kshutdown/wiki/Kiosk/\">Kiosk</a>") + "</qt>", InfoWidget::InfoType);
	mainLayout->addWidget(kioskInfo);
	
	updateWidgets(m_enablePassword->checkState() == Qt::Checked);
}

PasswordPreferences::~PasswordPreferences() {
	U_DEBUG << "PasswordPreferences::~PasswordPreferences()" U_END;
}

void PasswordPreferences::apply() {
	U_DEBUG << "PasswordPreferences::apply()" U_END;
	
	Config *config = Config::user();
	config->beginGroup("Password Protection");
	if (m_enablePassword->checkState() != Qt::Checked)
		config->write("Hash", "");

	int count = m_userActionList->count();
	for (int i = 0; i < count; i++) {
		auto *item = static_cast<QListWidgetItem *>(m_userActionList->item(i));
		QString key = item->data(m_configKeyRole).toString();
		config->write(key, item->checkState() == Qt::Checked);
	}

	config->endGroup();
}

// private:

QListWidgetItem *PasswordPreferences::addItem(const QString &key, const QString &text, const QIcon &icon) {
	QListWidgetItem *item = new QListWidgetItem(text, m_userActionList);
	item->setCheckState(
		Config::readBool("Password Protection", key, false)
		? Qt::Checked
		: Qt::Unchecked
	);
	item->setData(m_configKeyRole, key);
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setIcon(icon);
	
	return item;
}

void PasswordPreferences::updateWidgets(const bool passwordEnabled) {
	m_userActionList->setEnabled(passwordEnabled);
}

// private slots:

void PasswordPreferences::onEnablePassword(int state) {
	if (state == Qt::Checked) {
		QPointer<PasswordDialog> dialog = new PasswordDialog(this);
		if (dialog->exec() == PasswordDialog::Accepted)
			dialog->apply();
		else
			m_enablePassword->setCheckState(Qt::Unchecked);
		delete dialog;
	}
	updateWidgets(m_enablePassword->checkState() == Qt::Checked);
}
