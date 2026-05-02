#include "help_window.h"
#include "ui_help_window.h"

#include <QCoreApplication>
#include <QDebug>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QLayout>
#include <QSettings>

namespace {
constexpr auto kSettingsOrg = "HSE";
constexpr auto kSettingsApp = "FCM";
constexpr auto kRuLanguage = "RU";
constexpr auto kEnglishCode = "en";
constexpr auto kRussianCode = "ru";
constexpr auto kEnglishNamespace = "en.fcm.help";
constexpr auto kRussianNamespace = "ru.fcm.help";
}

HelpWindow::HelpWindow(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::HelpWindow),
      helpEngine(nullptr) {
    ui->setupUi(this);
    retranslate();
}

HelpWindow::~HelpWindow() {
    delete helpEngine;
    delete ui;
}

void HelpWindow::retranslate() {
    ui->retranslateUi(this);

    const bool isRussian = currentLanguageCode() == QLatin1String(kRussianCode);
    setWindowTitle(isRussian ? QString::fromUtf8("Справка") : QStringLiteral("Help"));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->contentTab), tr("Contents"));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->indexTab), tr("Index"));

    reloadHelpEngine();
}

QString HelpWindow::currentLanguageCode() const {
    const QSettings settings(kSettingsOrg, kSettingsApp);
    return settings.value("language", "").toString() == QLatin1String(kRuLanguage)
               ? QLatin1String(kRussianCode)
               : QLatin1String(kEnglishCode);
}

QString HelpWindow::currentNamespace() const {
    return currentLanguageCode() == QLatin1String(kRussianCode)
               ? QLatin1String(kRussianNamespace)
               : QLatin1String(kEnglishNamespace);
}

QUrl HelpWindow::defaultPageUrl() const {
    return QUrl(QStringLiteral("qthelp://%1/doc/model_settings.html").arg(currentNamespace()));
}

void HelpWindow::reloadHelpEngine() {
    while (QLayoutItem *item = ui->contentTab->layout()->takeAt(0)) {
        delete item;
    }
    while (QLayoutItem *item = ui->indexTab->layout()->takeAt(0)) {
        delete item;
    }

    delete helpEngine;
    helpEngine = nullptr;
    ui->textBrowser->setHelpEngine(nullptr);
    ui->textBrowser->clear();

    const QString helpDir = QCoreApplication::applicationDirPath() + "/help/" + currentLanguageCode();
    const QString qhcPath = helpDir + "/help.qhc";

    helpEngine = new QHelpEngine(qhcPath, this);
    if (!helpEngine->setupData()) {
        qWarning() << "Help setup error:" << helpEngine->error();
        return;
    }

    auto *content = helpEngine->contentWidget();
    auto *index = helpEngine->indexWidget();

    ui->contentTab->layout()->addWidget(content);
    ui->indexTab->layout()->addWidget(index);
    ui->textBrowser->setHelpEngine(helpEngine);

    connect(content, &QHelpContentWidget::linkActivated, this, &HelpWindow::showHelpPage);
    connect(index, &QHelpIndexWidget::linkActivated,
            this, [this](const QUrl &url, const QString &) { showHelpPage(url); });

    const auto docs = helpEngine->registeredDocumentations();
    if (docs.contains(currentNamespace())) {
        showHelpPage(defaultPageUrl());
    }
}

void HelpWindow::showHelpPage(const QUrl &url) {
    ui->textBrowser->setSource(url);
}
