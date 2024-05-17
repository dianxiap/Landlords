#include "scorepanel.h"
#include "ui_scorepanel.h"

ScorePanel::ScorePanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScorePanel)
{
    ui->setupUi(this);
}

ScorePanel::~ScorePanel()
{
    delete ui;
}

void ScorePanel::setScores(int left, int right, int user)
{
    // QString::number将整数转成QString类型
    ui->leftScore->setText(QString::number(left));
    ui->rightScore->setText(QString::number(right));
    ui->meScore->setText(QString::number(user));
}
