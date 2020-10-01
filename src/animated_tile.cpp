/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file animated_tile.cpp Everything related to animated tiles. */

#include "stdafx.h"
#include "core/alloc_func.hpp"
#include "core/smallvec_type.hpp"
#include "tile_cmd.h"
#include "viewport_func.h"
#include "framerate_type.h"

#include "safeguards.h"

/** The table/list with animated tiles. */
std::vector<TileIndex> _animated_tiles;

/**
 * Removes the given tile from the animated tile table.
 * @param tile the tile to remove
 */
void DeleteAnimatedTile(TileIndex tile)
{
	auto to_remove = std::find(_animated_tiles.begin(), _animated_tiles.end(), tile);
	if (to_remove != _animated_tiles.end()) {
		/* The order of the remaining elements must stay the same, otherwise the animation loop may miss a tile. */
		_animated_tiles.erase(to_remove);
		MarkTileDirtyByTile(tile, VMDF_NOT_MAP_MODE);
	}
}

/**
 * Add the given tile to the animated tile table (if it does not exist
 * on that table yet). Also increases the size of the table if necessary.
 * @param tile the tile to make animated
 */
void AddAnimatedTile(TileIndex tile)
{
	MarkTileDirtyByTile(tile, VMDF_NOT_MAP_MODE);
	include(_animated_tiles, tile);
}

/**
 * Animate all tiles in the animated tile list, i.e.\ call AnimateTile on them.
 */
void AnimateAnimatedTiles()
{
	extern void AnimateTile_Town(TileIndex tile);
	extern void AnimateTile_Station(TileIndex tile);
	extern void AnimateTile_Industry(TileIndex tile);
	extern void AnimateTile_Object(TileIndex tile);

	PerformanceAccumulator framerate(PFE_GL_LANDSCAPE);

	const TileIndex *ti = _animated_tiles.data();
	while (ti < _animated_tiles.data() + _animated_tiles.size()) {
		const TileIndex curr = *ti;
		switch (GetTileType(curr)) {
			case MP_HOUSE:
				AnimateTile_Town(curr);
				break;

			case MP_STATION:
				AnimateTile_Station(curr);
				break;

			case MP_INDUSTRY:
				AnimateTile_Industry(curr);
				break;

			case MP_OBJECT:
				AnimateTile_Object(curr);
				break;

			default:
				NOT_REACHED();
		}

		/* During the AnimateTile call, DeleteAnimatedTile could have been called,
		 * deleting an element we've already processed and pushing the rest one
		 * slot to the left. We can detect this by checking whether the index
		 * in the current slot has changed - if it has, an element has been deleted,
		 * and we should process the current slot again instead of going forward.
		 * NOTE: this will still break if more than one animated tile is being
		 *       deleted during the same AnimateTile call, but no code seems to
		 *       be doing this anyway.
		 */
		if (*ti == curr) ++ti;
	}
}

/**
 * Initialize all animated tile variables to some known begin point
 */
void InitializeAnimatedTiles()
{
	_animated_tiles.clear();
}
