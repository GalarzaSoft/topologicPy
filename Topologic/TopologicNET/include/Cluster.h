// This file is part of Topologic software library.
// Copyright(C) 2019, Cardiff University and University College London
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <Topology.h>

#include <TopologicCore/include/Cluster.h>

namespace Topologic
{
	ref class Vertex;
	ref class Edge;
	ref class Wire;
	ref class Face;
	ref class Shell;
	ref class Cell;
	ref class CellComplex;

	/// <summary>
	/// A Cluster is a collection of any topologic entities. It may be contiguous or not and may be manifold or non-manifold. Clusters can be nested within other Clusters.
	/// </summary>
	public ref class Cluster : Topology
	{
	public:
		/// <summary>
		/// Creates a Cluster by a set of Topologies. 
		/// </summary>
		/// <param name="topologies">A set of Topologies</param>
		/// <returns name="Cluster">The created Cluster</returns>
		static Cluster^ ByTopologies(System::Collections::Generic::IList<Topology^>^ topologies);

		/// <summary>
		/// Adds Topology to a Cluster and returns the resulting Topology.
		/// </summary>
		/// <param name="topology">A Topology</param>
		/// <returns>The created Topology</returns>
#ifdef TOPOLOGIC_DYNAMO
		[IsVisibleInDynamoLibrary(false)]
#endif
		Cluster^ AddTopology(Topology^ topology);

		/// <summary>
		/// Removes Topology from a Cluster and returns the resulting Topology.
		/// </summary>
		/// <param name="topology">A Topology</param>
		/// <returns>The created Topology</returns>
#ifdef TOPOLOGIC_DYNAMO
		[IsVisibleInDynamoLibrary(false)]
#endif
		Cluster^ RemoveTopology(Topology^ topology);

		/// <summary>
		/// Creates a geometry from Cluster.
		/// </summary>
		/// <returns>The created geometry</returns>
		property Object^ BasicGeometry
		{
			virtual Object^ get() override;
		}

		/// <summary>
		/// Returns the Shells constituent to the Cluster.
		/// </summary>
		/// <returns name="Shell[]">A list of Shells constituent to the Cluster</returns>
		property IList<Shell^>^ Shells
		{
			IList<Shell^>^ get();
		}

		/// <summary>
		/// Returns the Faces constituent to the Cluster.
		/// </summary>
		/// <returns name="Face[]">A list of Faces constituent to the Cluster</returns>
		property IList<Face^>^ Faces
		{
			IList<Face^>^ get();
		}

		/// <summary>
		/// Returns the Wires constituent to the Cluster.
		/// </summary>
		/// <returns name="Wire[]">A list of Wires constituent to the Cluster</returns>
		property IList<Wire^>^ Wires
		{
			IList<Wire^>^ get();
		}

		/// <summary>
		/// Returns the Edges constituent to the Cluster.
		/// </summary>
		/// <returns name="Edge[]">A list of Edges constituent to the Cluster</returns>
		property IList<Edge^>^ Edges
		{
			IList<Edge^>^ get();
		}

		/// <summary>
		/// Returns the Vertices constituent to the Cluster.
		/// </summary>
		/// <returns name="Vertex[]">A list of Vertices constituent to the Cluster</returns>
		property IList<Vertex^>^ Vertices
		{
			IList<Vertex^>^ get();
		}

		/// <summary>
		/// Returns the Cells constituent to the Cluster.
		/// </summary>
		/// <returns name="Cell[]">A list of Cells constituent to the Cluster</returns>
		property IList<Cell^>^ Cells
		{
			IList<Cell^>^ get();
		}

		/// <summary>
		/// Returns the CellComplexes constituent to the Cluster.
		/// </summary>
		/// <returns name="CellComplex[]">A list of CellComplexes constituent to the Cluster</returns>
		property IList<CellComplex^>^ CellComplexes
		{
			IList<CellComplex^>^ get();
		}

		/// <summary>
		/// Returns the type associated to Cluster.
		/// </summary>
		/// <returns>The type associated to Cluster</returns>
		static int Type();

	public protected:
		Cluster();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="kpCoreCluster"></param>
		Cluster(const std::shared_ptr<TopologicCore::Cluster>& kpCoreCluster);

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		virtual std::shared_ptr<TopologicCore::TopologicalQuery> GetCoreTopologicalQuery() override;

	protected:
		virtual ~Cluster();

		/// <summary>
		/// 
		/// </summary>
		std::shared_ptr<TopologicCore::Cluster>* m_pCoreCluster;
	};
}