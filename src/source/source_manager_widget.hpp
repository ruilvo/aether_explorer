/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#pragma once

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QString>
#include <QWidget>

class SourceManager;

class SourceManagerWidget : public QWidget
{
  public:
    SourceManagerWidget() = delete;
    SourceManagerWidget(SourceManager *manager);
    SourceManagerWidget(const SourceManagerWidget &) = delete;
    SourceManagerWidget &operator=(const SourceManagerWidget &) = delete;
    ~SourceManagerWidget() override = default;

  private:
    SourceManager *manager_;
    QComboBox *sourcesAvailableComboBox_;
    QDoubleSpinBox *centreFrequencySpinBox_;
    QPushButton *startStopPushButton_;
  private slots:
    void sourcesAvailableComboBoxTextChanged(const QString &text);
    void centreFrequencySpinBoxValueChanged(double d);
    void startStopPushButtonToggled(bool checked);
};
