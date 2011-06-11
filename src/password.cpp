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

#ifdef KS_PURE_QT
	#include <QPointer>
#endif // KS_PURE_QT

#include <QCheckBox>
#include <QCryptographicHash>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

// PasswordDialog

// public:

PasswordDialog::PasswordDialog(QWidget *parent, const bool newPasswordMode) :
	U_DIALOG(parent),
	m_newPasswordMode(newPasswordMode) {
	U_DEBUG << "PasswordDialog::PasswordDialog()" U_END;

	if (newPasswordMode)
		setWindowTitle(i18n("Enter New Password"));
	else
		setWindowTitle("KShutdown");

	QFormLayout *mainLayout = new QFormLayout();

	if (!newPasswordMode) {
		m_caption = new QLabel();
		mainLayout->addRow(m_caption);
	}
	else {
		m_caption = 0;
	}

	m_password = new QLineEdit();
	m_password->setEchoMode(QLineEdit::Password);
	connect(
		m_password, SIGNAL(textEdited(const QString &)),
		SLOT(onPasswordChange(const QString &))
	);
	mainLayout->addRow(i18n("Password:"), m_password);
	
	if (newPasswordMode) {
		m_caption = new QLabel();
		mainLayout->addRow(m_caption);

		m_confirmPassword = new QLineEdit();
		m_confirmPassword->setEchoMode(QLineEdit::Password);
		connect(
			m_confirmPassword, SIGNAL(textEdited(const QString &)),
			SLOT(onConfirmPasswordChange(const QString &))
		);
		mainLayout->addRow(i18n("Confirm Password:"), m_confirmPassword);
		//!!!sep
		m_status = new InfoWidget(this);
		m_status->setText(
			"<qt>" +
			i18n("The password will be saved as SHA-1 hash.") + "<br>" +
			i18n("Short password can be easily cracked.") +
			"</qt>",
			InfoWidget::WarningType
		);
		mainLayout->addRow(m_status);//!!!add?
	}
	else {
		m_confirmPassword = 0;
		m_status = 0;
	}

// TODO: common code (abstract UDialog class)
	U_DIALOG_BUTTON_BOX *dialogButtonBox;
#ifdef KS_NATIVE_KDE
	dialogButtonBox = new U_DIALOG_BUTTON_BOX(this);
	m_acceptButton = dialogButtonBox->addButton(KStandardGuiItem::ok(), U_DIALOG_BUTTON_BOX::AcceptRole);
	dialogButtonBox->addButton(KStandardGuiItem::cancel(), U_DIALOG_BUTTON_BOX::RejectRole);
#else
	dialogButtonBox = new U_DIALOG_BUTTON_BOX(U_DIALOG_BUTTON_BOX::Ok | U_DIALOG_BUTTON_BOX::Cancel);
	m_acceptButton = dialogButtonBox->button(U_DIALOG_BUTTON_BOX::Ok);//!!!test
#endif // KS_NATIVE_KDE
	connect(dialogButtonBox, SIGNAL(accepted()), SLOT(accept()));
	connect(dialogButtonBox, SIGNAL(rejected()), SLOT(reject()));
	mainLayout->addRow(dialogButtonBox);
	
	setLayout(mainLayout);
}

/*!!!
changing settings by a password
shutdown,hib,etc.
*/

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

	QPointer<PasswordDialog> dialog = new PasswordDialog(parent, false);
	dialog->m_caption->setText(i18n("Enter password to perform action: %0").arg(caption));
	
	bool ok = false;
	if (dialog->exec() == PasswordDialog::Accepted) {
		if (hash != toHash(dialog->m_password->text())) {
			U_ERROR_MESSAGE(parent, i18n("Invalid password"));
		}
		else {
			ok = true;
		}
	}
	delete dialog;

	return ok;
}

QString PasswordDialog::toHash(const QString &password) {
	if (password.isEmpty())
		return "";
	
	QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha1);
	
	return QString(hash.toHex());
}

// private:

void PasswordDialog::updateStatus() {
	if (!m_newPasswordMode)
		return;
	
	QString text = QString::null;
	
	bool ok = (m_password->text() == m_confirmPassword->text());
	
	if (!ok)
		text = i18n("Confirmation password is different");
	
	m_acceptButton->setEnabled(ok);
	m_status->setText(text, InfoWidget::ErrorType);
	resize(sizeHint());
}

// private slots:

void PasswordDialog::onConfirmPasswordChange(const QString &text) {
	updateStatus();
}

void PasswordDialog::onPasswordChange(const QString &text) {
	updateStatus();
}

// PasswordPreferences

// public:

PasswordPreferences::PasswordPreferences(QWidget *parent) :
	QWidget(parent) {
	U_DEBUG << "PasswordPreferences::PasswordPreferences()" U_END;

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	
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
	foreach (Action *action, MainWindow::self()->actionHash().values()) {
		QListWidgetItem *item = new QListWidgetItem(action->originalText(), m_userActionList);
		QString key = "kshutdown/action/" + action->id();
		item->setCheckState(
			Config::readBool("Password Protection", key, false)
			? Qt::Checked
			: Qt::Unchecked
		);//!!!common code
		item->setData(Qt::ToolTipRole, key);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	}
	
	userActionListLabel->setBuddy(m_userActionList);
	mainLayout->addWidget(m_userActionList);
	
	mainLayout->setMargin(5);
	mainLayout->setSpacing(5);
	
	InfoWidget *kioskInfo = new InfoWidget(this);
	kioskInfo->setText("<qt>" + i18n("See Also: %0").arg("<a href=\"http://sourceforge.net/apps/mediawiki/kshutdown/index.php?title=Kiosk\">Kiosk</a>") + "</qt>", InfoWidget::InfoType);
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
	config->endGroup();
	config->sync();
}

// private:

void PasswordPreferences::updateWidgets(const bool passwordEnabled) {
	m_userActionList->setEnabled(passwordEnabled);
}

// private slots:

void PasswordPreferences::onEnablePassword(int state) {
	if (state == Qt::Checked) {
		QPointer<PasswordDialog> dialog = new PasswordDialog(this, true);
		if (dialog->exec() == PasswordDialog::Accepted)
			dialog->apply();
		else
			m_enablePassword->setCheckState(Qt::Unchecked);
		delete dialog;
	}
	updateWidgets(m_enablePassword->checkState() == Qt::Checked);
}
//!!!review