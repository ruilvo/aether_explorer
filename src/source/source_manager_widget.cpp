/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "source_manager_widget.hpp"

#include "source_manager.hpp"

#include <QDebug>
#include <QFormLayout>
#include <QStringList>

SourceManagerWidget::SourceManagerWidget(SourceManager *manager)
    : QWidget(nullptr), manager_(manager), sourcesAvailableComboBox_(new QComboBox(this)),
      centreFrequencySpinBox_(new QDoubleSpinBox(this)),
      startStopPushButton_(new QPushButton(this)), layout_(new QVBoxLayout(this))
{
    startStopPushButton_->setCheckable(true);
    startStopPushButton_->setText("Start");

    layout_->addWidget(startStopPushButton_);

    auto *formLayout = new QFormLayout();

    formLayout->addRow("Source", sourcesAvailableComboBox_);
    formLayout->addRow("Centre freq.", centreFrequencySpinBox_);

    layout_->addLayout(formLayout);

    if (manager_)
    {
        auto sourceFactory = manager_->getSourceFactory();
        if (sourceFactory)
        {
            auto sourcesAvailable = sourceFactory->getNames();
            for (const auto &sourceName : sourcesAvailable)
            {
                sourcesAvailableComboBox_->addItem(QString::fromStdString(sourceName));
            }
        }
    }

    connect(startStopPushButton_, &QPushButton::toggled, this,
            &SourceManagerWidget::startStopPushButtonToggled);
    connect(sourcesAvailableComboBox_, &QComboBox::currentTextChanged, this,
            &SourceManagerWidget::sourcesAvailableComboBoxTextChanged);
    connect(centreFrequencySpinBox_, &QDoubleSpinBox::valueChanged, this,
            &SourceManagerWidget::centreFrequencySpinBoxValueChanged);

    if (sourcesAvailableComboBox_->count() > 0)
    {
        sourcesAvailableComboBoxTextChanged(sourcesAvailableComboBox_->currentText());
    }
};

void SourceManagerWidget::startStopPushButtonToggled(bool checked)
{
    if (manager_)
    {
        auto *source = manager_->getSource();
        if (checked && source)
        {
            source->start();
            centreFrequencySpinBox_->blockSignals(true);
            centreFrequencySpinBox_->setValue(source->getCentreFrequency());
            centreFrequencySpinBox_->blockSignals(false);
        }
        else if (!checked && source)
        {
            source->stop();
        }
    }
    sourcesAvailableComboBox_->setEnabled(!checked);
    startStopPushButton_->setText(checked ? "Stop" : "Start");
}

void SourceManagerWidget::sourcesAvailableComboBoxTextChanged(const QString &text)
{
    if (manager_)
    {
        auto *source = manager_->getSource();
        if (source)
        {
            layout_->removeWidget(source->getWidget());
        }
        manager_->setSource(text.toStdString());
        source = manager_->getSource();
        if (source)
        {
            layout_->addWidget(source->getWidget());
        }
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
