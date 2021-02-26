/*
 * This file is part of Aether Explorer
 *
 * Copyright (c) 2021 Rui Oliveira
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Consult LICENSE.txt for detailed licensing information
 */

#include "ISource.hpp"
#include "soapysdr_radio.hpp"
#include "source_factory.hpp"
#include "source_listeners_collection.hpp"
#include "source_manager.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto listenersCollection = SourceListenersCollection();
    /*
     * Now initialize the listeners...
     */

    auto sourceFactory = SourceFactory();
    /*
     * Now register sources...
     */
    sourceFactory.registerSource("SoapySDR",
                                []() { return std::make_unique<SoapySdrRadio>(); });

    auto sourceManager =
        SourceManager(std::move(sourceFactory), std::move(listenersCollection));

    // In the future this would be part of a larger main Window, of course...
    sourceManager.getWidget()->show();

    return app.exec();
}
