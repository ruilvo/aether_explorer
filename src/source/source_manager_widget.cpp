/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "source_manager_widget.hpp"

#include "source_manager.hpp"

#include <QStringList>
#include <QVBoxLayout>

SourceManagerWidget::SourceManagerWidget(SourceManager *manager)
    : QWidget(nullptr), manager_(manager), sourcesAvailableComboBox_(new QComboBox(this)),
      centreFrequencySpinBox_(new QDoubleSpinBox(this)),
      startStopPushButton_(new QPushButton(this))
{
    startStopPushButton_->setCheckable(true);
    startStopPushButton_->setText("Start");

    auto *layout = new QVBoxLayout(this);

    layout->addWidget(startStopPushButton_);
    layout->addWidget(sourcesAvailableComboBox_);
    layout->addWidget(centreFrequencySpinBox_);

    if (manager_)
    {
        auto sourcesAvailable = manager_->getSourceFactory()->getNames();
        for (const auto &sourceName : sourcesAvailable)
        {
            sourcesAvailableComboBox_->addItem(QString::fromStdString(sourceName));
        }
    }

    connect(startStopPushButton_, &QPushButton::toggled, this,
            &SourceManagerWidget::startStopPushButtonToggled);
    connect(sourcesAvailableComboBox_, &QComboBox::currentTextChanged, this,
            &SourceManagerWidget::sourcesAvailableComboBoxTextChanged);
    connect(centreFrequencySpinBox_, &QDoubleSpinBox::valueChanged, this,
            &SourceManagerWidget::centreFrequencySpinBoxValueChanged);
};

void SourceManagerWidget::startStopPushButtonToggled(bool checked)
{
    if (manager_)
    {
        auto *source = manager_->getSource();
        if (checked && source)
        {
            source->start();
        }
        else if (!checked && source)
        {
            source->stop();
        }
    }
    sourcesAvailableComboBox_->setEnabled(!checked);
}

void SourceManagerWidget::sourcesAvailableComboBoxTextChanged(const QString &text)
{
    if (manager_)
    {
        manager_->setSource(text.toStdString());
    }
}

void SourceManagerWidget::centreFrequencySpinBoxValueChanged(double d)
{
    if (manager_)
    {
        auto *source = manager_->getSource();
        if (source)
        {
            source->setCentreFrequency(d);
        }
    }
}
