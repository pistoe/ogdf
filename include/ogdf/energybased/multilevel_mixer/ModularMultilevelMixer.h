/** \file
 * \brief MMM is a Multilevel Graph drawing Algorithm that can use different modules.
 *
 * \author Gereon Bartel
 *
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 *
 * \par
 * Copyright (C)<br>
 * See README.md in the OGDF root directory for details.
 *
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation;
 * see the file LICENSE.txt included in the packaging of this file
 * for details.
 *
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * \par
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, see
 * http://www.gnu.org/copyleft/gpl.html
 */

#pragma once

#include <ogdf/basic/LayoutModule.h>
#include <ogdf/basic/basic.h>
#include <ogdf/energybased/multilevel_mixer/InitialPlacer.h>
#include <ogdf/energybased/multilevel_mixer/MultilevelBuilder.h>

#include <memory>

namespace ogdf {
class GraphAttributes;
class MultilevelGraph;

//! Modular multilevel graph layout.
/**
 * @ingroup gd-multi
 *
 * <H3>%Module options</H3>
 * The various phases of the algorithm can be exchanged by setting
 * module options allowing flexible customization. The algorithm provides
 * the following module options:
 *
 * <table>
 *   <tr>
 *     <th><i>Option</i><th><i>Type</i><th><i>Default</i><th><i>Description</i>
 *   </tr><tr>
 *     <td><i>multilevelBuilder</i><td>MultilevelBuilder<td>SolarMerger
 *     <td>The multilevel builder module that computes the multilevel graph hierarchy.
 *   </tr><tr>
 *     <td><i>initialPlacer</i><td>InitialPlacer<td>BarycenterPlacer
 *     <td>The initial placer module that computes the initial positions for nodes inserted into the previous level.
 *   </tr><tr>
 *     <td><i>levelLayout</i><td>LayoutModule<td>FastMultipoleEmbedder
 *     <td>The layout module applied on each level.
 *   </tr><tr>
 *     <td><i>finalLayout</i><td>LayoutModule<td>none
 *     <td>The layout module applied on the last level.
 *   </tr><tr>
 *     <td><i>postLayout</i><td>LayoutModule<td>none
 *     <td>The layout module applied to the final drawing for additional beautification.
 *   </tr>
 * </table>
 */
class OGDF_EXPORT ModularMultilevelMixer : public LayoutModule {
private:
	//! The layout algorithm applied on each level.
	/**
	 * The one-level layout module should not completely discard the initial Layout
	 * but do incremental beautification.
	 * Usually a simple force-directed / energy-based Layout should be chosen.
	 */
	std::unique_ptr<LayoutModule> m_oneLevelLayoutModule;

	//! The layout algorithm applied on the last level (i.e., the largest graph in the multilevel hierarchy).
	/**
	 * The final layout module can be set to speed up the computation if the
	 * one-level layout ist relatively slow. If not set, the one-level layout
	 * is also used on the last level.
	 */
	std::unique_ptr<LayoutModule> m_finalLayoutModule;

	//! The multilevel builder module computes the multilevel hierarchy.
	std::unique_ptr<MultilevelBuilder> m_multilevelBuilder;

	//! The initial placer module computes the initial positions for nodes inserted into the previous level.
	std::unique_ptr<InitialPlacer> m_initialPlacement;

	//! The one-level layout will be called #m_times to improve quality.
	int m_times;

	//! If set to a value > 0, all edge weights will be set to this value.
	double m_fixedEdgeLength;

	//! If set to a value > 0, all node sizes will be set to this value.
	double m_fixedNodeSize;

	double m_coarseningRatio; //!< Ratio between sizes of previous (p) and current (c) level graphs: c/p

	bool m_levelBound; //!< Determines if computation is stopped when number of levels is too high.
	bool m_randomize; //!< Determines if initial random layout is computed.

public:
	//! Error codes for calls.
	enum class erc {
		None, //!< no error
		LevelBound //!< level bound exceeded by merger step
	};

	ModularMultilevelMixer();

	//! Sets the one-level layout module to \p levelLayout.
	void setLevelLayoutModule(LayoutModule* levelLayout) {
		m_oneLevelLayoutModule.reset(levelLayout);
	}

	//! Sets the final layout module to \p finalLayout.
	void setFinalLayoutModule(LayoutModule* finalLayout) { m_finalLayoutModule.reset(finalLayout); }

	//! Sets the multilevel builder module to \p levelBuilder.
	void setMultilevelBuilder(MultilevelBuilder* levelBuilder) {
		m_multilevelBuilder.reset(levelBuilder);
	}

	//! Sets the initial placer module to \p placement.
	void setInitialPlacer(InitialPlacer* placement) { m_initialPlacement.reset(placement); }

	//! Determines how many times the one-level layout will be called.
	void setLayoutRepeats(int times = 1) { m_times = times; }

	//! If \p len > 0, all edge weights will be set to \p len.
	void setAllEdgeLengths(double len) { m_fixedEdgeLength = len; }

	//! If \p size > 0, all node sizes will be set to \p size.
	void setAllNodeSizes(double size) { m_fixedNodeSize = size; }

	//! Determines if an initial random layout is computed.
	void setRandomize(bool b) { m_randomize = b; }

	//! Determines if computation is stopped when number of levels is too high.
	void setLevelBound(bool b) { m_levelBound = b; }

	//! Calls the multilevel layout algorithm for graph attributes \p GA.
	void call(GraphAttributes& GA) override;

	/**
	 * \brief Calls the multilevel layout algorithm for multilevel graph \a MLG.
	 *
	 * This method allows the mixer to modify the Graph, saving some memory
	 * compared to a normal call(GA) in our implementation.
	 * (because the Graph is already given in the MultiLevelGraph Format
	 *   (or can be converted without creating a copy) AND the layout would need a copy otherwise).
	 * All Incremental Layouts (especially energy based) CAN be called by ModularMultilevelMixer.
	 * @param MLG is the input graph and will also be assigned the layout information.
	 */
#if 0
	virtual void call(MultilevelGraph &MLG) {
		GraphAttributes GA(MLG.getGraph());
		MLG.exportAttributesSimple(GA);
		call(GA);
		MLG.importAttributesSimple(GA);
	};
#else
	virtual void call(MultilevelGraph& MLG);
#endif

	//! Returns the error code of last call.
	erc errorCode() { return m_errorCode; }

	//! Returns the ratio c/p between sizes of previous (p) and current (c) level graphs.
	double coarseningRatio() { return m_coarseningRatio; }

private:
	erc m_errorCode; //!< The error code of the last call.
};

}
