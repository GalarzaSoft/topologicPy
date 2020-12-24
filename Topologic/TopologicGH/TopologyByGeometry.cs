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

﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Grasshopper.Kernel;
using Grasshopper.Kernel.Types;
using Rhino.Geometry;
using Rhino.Geometry.Collections;
using Topologic;

namespace TopologicGH
{
    public class TopologyByGeometry : GH_Component
    {
        public TopologyByGeometry()
          : base("Topology.ByGeometry", "Topology.ByGeometry", "Creates a Topology by geometry.", "Topologic", "Topology")
        {
        }

        /// <summary>
        /// Registers all the input parameters for this component.
        /// </summary>
        protected override void RegisterInputParams(GH_Component.GH_InputParamManager pManager)
        {
            pManager.AddGeometryParameter("Geometry", "Geometry", "Geometry", GH_ParamAccess.item);
            pManager.AddNumberParameter("Tolerance", "Tolerance", "Tolerance", GH_ParamAccess.item, 0.0001);
        }

        /// <summary>
        /// Registers all the output parameters for this component.
        /// </summary>
        protected override void RegisterOutputParams(GH_Component.GH_OutputParamManager pManager)
        {
            pManager.AddGenericParameter("Topology", "Topology", "Topology", GH_ParamAccess.item);
        }

        /// <summary>
        /// This is the method that actually does the work.
        /// </summary>
        /// <param name="DA">The DA object is used to retrieve from inputs and store in outputs.</param>
        protected override void SolveInstance(IGH_DataAccess DA)
        {
            Object ghGeometryBase = null;
            double tolerance = 0.0001;
            if (!DA.GetData(0, ref ghGeometryBase)) { return; }
            if (!DA.GetData(1, ref tolerance)) { return; }

            if (ghGeometryBase == null) { return; }
            Type type = ghGeometryBase.GetType();

            Topologic.Topology topology = null;
            GH_Point ghPoint = ghGeometryBase as GH_Point;
            if (ghPoint != null)
            {
                topology = ByPoint(ghPoint.Value);
                DA.SetData(0, topology);
                return;
            }

            GH_Line ghLine = ghGeometryBase as GH_Line;
            if (ghLine != null)
            {
                topology = ByLine(ghLine.Value);
                DA.SetData(0, topology);
                return;
            }

            GH_Curve ghCurve = ghGeometryBase as GH_Curve;
            if (ghCurve != null)
            {
                topology = ByCurve(ghCurve.Value);
                DA.SetData(0, topology);
                return;
            }

            GH_Surface ghSurface = ghGeometryBase as GH_Surface;
            if (ghSurface != null)
            {
                //topology = ByBrep(ghSurface.Value.Faces[0].ToBrep(), tolerance);
                //topology = ByBrepFace(ghSurface.Value.Faces[0]);
                topology = ByBrep(ghSurface.Value, tolerance);
                DA.SetData(0, topology);
                return;
            }

            GH_Brep ghBrep = ghGeometryBase as GH_Brep;
            if (ghBrep != null)
            {
                topology = ByBrep(ghBrep.Value, tolerance);
                DA.SetData(0, topology);
                return;
            }

            GH_Box ghBox = ghGeometryBase as GH_Box;
            if (ghBox != null)
            {
                topology = ByBox(ghBox.Value);
                DA.SetData(0, topology);
                return;
            }

            GH_Mesh ghMesh = ghGeometryBase as GH_Mesh;
            if (ghMesh != null)
            {
                topology = ByMesh(ghMesh.Value);
                DA.SetData(0, topology);
                return;
            }

            //BrepLoop ghBrepLoop = ghGeometryBase as BrepLoop;
            //if (ghBrepLoop != null)
            //{
            //    topology = ByBrepLoop(ghBrepLoop);
            //    DA.SetData(0, topology);
            //    return;
            //}

            throw new Exception("This type of geometry is not yet supported.");
        }

        private Topologic.Topology ByBox(Box ghBox)
        {
            return ByBrep(ghBox.ToBrep(), 0.0001);
        }

        // https://developer.rhino3d.com/guides/cpp/brep-data-structure/
        private Topology ByBrep(Brep ghBrep, double tolerance)
        {
            BrepFaceList ghBrepFaces = ghBrep.Faces;
            List<Face> faces = new List<Face>();
            foreach (BrepFace ghBrepFace in ghBrepFaces)
            {
                Face face = ByBrepFace(ghBrepFace);
                faces.Add(face);
            }

            if (faces.Count == 0)
            {
                return null;
            } else if (faces.Count == 1)
            {
                return faces[0];
            }
            else
            {
                Shell shell = Shell.ByFaces(faces, tolerance);
                if(ghBrep.IsSolid)
                {
                    Cell cell = Cell.ByShell(shell);
                    return cell;
                }

                return shell;
            }
        }

        private Face ByBrepFace(BrepFace ghBrepFace)
        {
            Rhino.Geometry.Surface ghSurface = ghBrepFace.UnderlyingSurface();
            
            Face untrimmedFace = BySurface(ghSurface);

            BrepLoop ghOuterLoop = ghBrepFace.OuterLoop;
            Wire outerWire = null;
            BrepLoopList ghLoops = ghBrepFace.Loops;
            List<Wire> innerWires = new List<Wire>();
            foreach (BrepLoop ghLoop in ghLoops)
            {
                BrepTrimList ghTrims = ghLoop.Trims;
                List<Edge> trimmingEdges = new List<Edge>();
                foreach (BrepTrim ghTrim in ghTrims)
                {
                    BrepEdge ghEdge = ghTrim.Edge;
                    if (ghEdge == null)
                    {
                        continue;
                        //throw new Exception("An invalid Rhino edge is encountered.");
                    }

                    Topology topology = ByCurve(ghEdge.DuplicateCurve());

                    // Edge or Wire?
                    Edge trimmingEdge = topology as Edge;
                    if (trimmingEdge != null)
                    {
                        trimmingEdges.Add(trimmingEdge);
                    }

                    Wire partialTrimmingWire = topology as Wire;
                    if (partialTrimmingWire != null)
                    {
                        IList<Edge> partialTrimmingEdges = partialTrimmingWire.Edges;
                        trimmingEdges.AddRange(partialTrimmingEdges);
                    }
                }
                Wire trimmingWire = Wire.ByEdges(trimmingEdges);
                IList<Vertex> trimmingVertices = trimmingWire.Vertices;

                if (ghLoop == ghOuterLoop)
                {
                    outerWire = trimmingWire;
                }
                else
                {
                    innerWires.Add(trimmingWire);
                }
            }

            Face outerTrimmedFace = Topologic.Utilities.FaceUtility.TrimByWire(untrimmedFace, outerWire, true);
            Face finalFace = outerTrimmedFace.AddInternalBoundaries(innerWires);

            return finalFace;
        }

        private Topology ByMesh(Mesh ghMesh)
        {
            MeshVertexList ghMeshVertices = ghMesh.Vertices;
            int ghMeshVertexCount = ghMeshVertices.Count;
            MeshFaceList ghMeshFaces = ghMesh.Faces;
            int ghMeshFaceCount = ghMeshFaces.Count;

            List<global::Topologic.Vertex> vertices = new List<global::Topologic.Vertex>();
            for (int i = 0; i < ghMeshVertexCount; ++i)
            {
                Point3f ghPoint = ghMeshVertices[i];
                Vertex vertex = ByPoint(ghPoint);
                vertices.Add(vertex);
            }

            List<IList<int>> indices2D = new List<IList<int>>();
            for (int i = 0; i < ghMeshFaceCount; ++i)
            {
                MeshFace ghMeshFace = ghMeshFaces[i];
                List<int> indices1D = new List<int>();
                indices1D.Add(ghMeshFace.A);
                indices1D.Add(ghMeshFace.B);
                indices1D.Add(ghMeshFace.C);
                if (ghMeshFace.IsQuad)
                {
                    indices1D.Add(ghMeshFace.D);
                }
                indices1D.Add(ghMeshFace.A);

                indices2D.Add(indices1D);
            }

            IList<Topology> topologies = Topology.ByVerticesIndices(vertices, indices2D);

            Cluster cluster = Cluster.ByTopologies(topologies);
            Topology topology = cluster.SelfMerge();
            return topology;
        }

        //private Topology ByBrepLoop(BrepLoop ghBrepLoop)
        //{
        //    throw new NotImplementedException();
        //}

        public Topologic.Vertex ByPoint(Point3d ghPoint)
        {
            return Topologic.Vertex.ByCoordinates(ghPoint.X, ghPoint.Y, ghPoint.Z);
        }

        private Topologic.Topology ByCurve(Curve ghCurve)
        {
            LineCurve ghLine = ghCurve as LineCurve;
            if (ghLine != null)
            {
                return ByLine(ghLine.Line);
            }

            Rhino.Geometry.NurbsCurve ghNurbsCurve = ghCurve as Rhino.Geometry.NurbsCurve;
            if (ghNurbsCurve != null)
            {
                return ByNurbsCurve(ghNurbsCurve);
            }

            ArcCurve ghArcCurve = ghCurve as ArcCurve;
            if (ghArcCurve != null)
            {
                return ByArcCurve(ghArcCurve);
            }

            BrepEdge ghBrepEdge = ghCurve as BrepEdge;
            if (ghBrepEdge != null)
            {
                return ByBrepEdge(ghBrepEdge);
            }

            //BrepTrim ghBrepTrim = ghCurve as BrepTrim;
            //if (ghBrepTrim != null)
            //{
            //    return ByBrepTrim(ghBrepTrim);
            //}

            PolylineCurve ghPolylineCurve = ghCurve as PolylineCurve;
            if (ghPolylineCurve != null)
            {
                return ByPolylineCurve(ghPolylineCurve);
            }

            PolyCurve ghPolyCurve = ghCurve as PolyCurve;
            if (ghPolyCurve != null)
            {
                return ByPolyCurve(ghPolyCurve);
            }
            
            throw new Exception("This type of curve is not yet supported.");
        }

        private Topologic.Wire ByPolyCurve(PolyCurve ghPolyCurve)
        {
            Curve[] ghCurves = ghPolyCurve.Explode();
            List<Topologic.Edge> edges = new List<Topologic.Edge>();
            foreach(Curve ghCurve in ghCurves)
            {
                Topologic.Topology topology = ByCurve(ghCurve);

                Topologic.Edge edge = topology as Topologic.Edge;
                if(edge != null)
                {
                    edges.Add(edge);
                    continue;
                }

                Topologic.Wire wire = topology as Topologic.Wire;
                if(wire != null)
                {
                    edges.AddRange(wire.Edges);
                    continue;
                }
            }

            return Topologic.Wire.ByEdges(edges);
        }

        private Topologic.Edge ByBrepEdge(BrepEdge ghBrepEdge)
        {
            Rhino.Geometry.NurbsCurve ghNurbsCurve = ghBrepEdge.ToNurbsCurve();
            return ByNurbsCurve(ghNurbsCurve);
        }

        private Topologic.Edge ByArcCurve(ArcCurve ghArcCurve)
        {
            Rhino.Geometry.NurbsCurve ghNurbsCurve = ghArcCurve.ToNurbsCurve();
            return ByNurbsCurve(ghNurbsCurve);
        }

        private Topologic.Edge ByNurbsCurve(Rhino.Geometry.NurbsCurve ghNurbsCurve)
        {
            int degree = ghNurbsCurve.Degree;
            bool isClosed = ghNurbsCurve.IsClosed;
            bool isPeriodic = ghNurbsCurve.IsPeriodic;
            bool isRational = ghNurbsCurve.IsRational;
            NurbsCurveKnotList ghKnots = ghNurbsCurve.Knots;
            List<double> knots = ghKnots.ToList();
            
            // OCCT-compatible
            knots.Insert(0, knots[0]);
            knots.Add(knots.Last());

            NurbsCurvePointList ghControlPoints = ghNurbsCurve.Points;
            List<Topologic.Vertex> controlPoints = new List<Topologic.Vertex>();
            List<double> weights = new List<double>();
            for (int i = 0; i < ghControlPoints.Count; ++i)
            {
                controlPoints.Add(ByPoint(ghControlPoints[i].Location));
                weights.Add(ghControlPoints[i].Weight);
            }

            return Topologic.Edge.ByNurbsParameters(controlPoints, weights, knots, isRational, isPeriodic, degree);
        }

        private Topologic.Edge ByLine(Rhino.Geometry.Line ghLine)
        {
            Topologic.Vertex vertex1 = ByPoint(ghLine.From);
            Topologic.Vertex vertex2 = ByPoint(ghLine.To);
            return Topologic.Edge.ByStartVertexEndVertex(vertex1, vertex2);
        }

        private Topologic.Wire ByPolylineCurve(PolylineCurve ghPolylineCurve)
        {
            int numPoints = ghPolylineCurve.PointCount;
            if(numPoints < 1)
            {
                return null;
            }

            List<Topologic.Vertex> vertices = new List<Topologic.Vertex>();
            List<int> indices = new List<int>();
            for(int i = 0; i < numPoints; ++i)
            {
                Point3d ghPoint = ghPolylineCurve.Point(i);
                Topologic.Vertex vertex = ByPoint(ghPoint);
                vertices.Add(vertex);
                indices.Add(i);
            }

            if(ghPolylineCurve.IsClosed)
            {
                //vertices.Add(vertices[0]);
                //indices.Add(0);
                List<IList<int>> listOfIndices = new List<IList<int>>();
                listOfIndices.Add(indices);
                IList<Topology> topologyList = (IList<Topology>)Topologic.Topology.ByVerticesIndices(vertices, listOfIndices);
                IList<Wire> wireList = (IList<Wire>)topologyList[0].Wires;
                return wireList[0];
            }
            else
            {
                List<IList<int>> listOfIndices = new List<IList<int>>();
                listOfIndices.Add(indices);
                IList<Topology> topologyList = (IList<Topology>)Topologic.Topology.ByVerticesIndices(vertices, listOfIndices);
                return topologyList[0] as Topologic.Wire;
            }
        }

        private Topologic.Face BySurface(Rhino.Geometry.Surface ghSurface)
        {
            SumSurface ghSumSurface = ghSurface as SumSurface;
            if (ghSumSurface != null)
            {
                return BySumSurface(ghSumSurface);
            }

            RevSurface ghRevSurface = ghSurface as RevSurface;
            if (ghRevSurface != null)
            {
                return ByRevSurface(ghRevSurface);
            }

            PlaneSurface ghPlaneSurface = ghSurface as PlaneSurface;
            if (ghPlaneSurface != null)
            {
                return ByPlaneSurface(ghPlaneSurface);
            }

            //ClippingPlaneSurface ghClippingPlaneSurface = ghSurface as ClippingPlaneSurface;
            //if (ghClippingPlaneSurface != null)
            //{
            //    return ByClippingPlaneSurface(ghClippingPlaneSurface);
            //}

            Extrusion ghExtrusion = ghSurface as Extrusion;
            if (ghExtrusion != null)
            {
                return ByExtrusion(ghExtrusion);
            }

            Rhino.Geometry.NurbsSurface ghNurbsSurface = ghSurface as Rhino.Geometry.NurbsSurface;
            if (ghNurbsSurface != null)
            {
                return ByNurbsSurface(ghNurbsSurface);
            }

            //BrepFace ghBrepFace = ghSurface as BrepFace;
            //if (ghBrepFace != null)
            //{
            //    return ByBrepFace(ghBrepFace);
            //}

            throw new Exception("This type of surface is not yet supported.");
        }

        private Face ByNurbsSurface(Rhino.Geometry.NurbsSurface ghNurbsSurface)
        {
            int uDegree = ghNurbsSurface.Degree(0);
            int vDegree = ghNurbsSurface.Degree(1);
            bool isUClosed = ghNurbsSurface.IsClosed(0);
            bool isVClosed = ghNurbsSurface.IsClosed(1);
            bool isUPeriodic = ghNurbsSurface.IsPeriodic(0);
            bool isVPeriodic = ghNurbsSurface.IsPeriodic(1);
            bool isRational = ghNurbsSurface.IsRational;
            NurbsSurfaceKnotList ghUKnots = ghNurbsSurface.KnotsU;
            List<double> uKnots = ghUKnots.ToList();

            NurbsSurfaceKnotList ghVKnots = ghNurbsSurface.KnotsV;
            List<double> vKnots = ghVKnots.ToList();

            // OCCT-compatible
            uKnots.Insert(0, uKnots[0]);
            uKnots.Add(uKnots.Last());
            vKnots.Insert(0, vKnots[0]);
            vKnots.Add(vKnots.Last());

            NurbsSurfacePointList ghControlPoints = ghNurbsSurface.Points;
            List<IList<Topologic.Vertex>> controlPoints = new List<IList<Topologic.Vertex>>();
            List<IList<double>> weights = new List<IList<double>>();
            for (int i = 0; i < ghControlPoints.CountU; ++i)
            {
                List<Topologic.Vertex> controlPoints1D = new List<Topologic.Vertex>();
                List<double> weights1D = new List<double>();
                for (int j = 0; j < ghControlPoints.CountV; ++j)
                {
                    ControlPoint ghControlPoint = ghControlPoints.GetControlPoint(i, j);
                    controlPoints1D.Add(ByPoint(ghControlPoint.Location));
                    weights1D.Add(ghControlPoint.Weight);
                }
                controlPoints.Add(controlPoints1D);
                weights.Add(weights1D);
            }

            return Topologic.Face.ByNurbsParameters(controlPoints, weights, uKnots, vKnots, isRational, isUPeriodic, isVPeriodic, uDegree, vDegree);
        }

        private Face ByExtrusion(Extrusion ghExtrusion)
        {
            Rhino.Geometry.NurbsSurface ghNurbsSurface = ghExtrusion.ToNurbsSurface();
            return ByNurbsSurface(ghNurbsSurface);
        }

        private Topologic.Face ByPlaneSurface(PlaneSurface ghPlaneSurface)
        {
            Rhino.Geometry.NurbsSurface ghNurbsSurface = ghPlaneSurface.ToNurbsSurface();
            return ByNurbsSurface(ghNurbsSurface);
        }

        private Topologic.Face ByRevSurface(RevSurface ghRevSurface)
        {
            Rhino.Geometry.NurbsSurface ghNurbsSurface = ghRevSurface.ToNurbsSurface();
            return ByNurbsSurface(ghNurbsSurface);
        }

        private Topologic.Face BySumSurface(SumSurface ghSumSurface)
        {
            Rhino.Geometry.NurbsSurface ghNurbsSurface = ghSumSurface.ToNurbsSurface();
            return ByNurbsSurface(ghNurbsSurface);
        }

        //Topologic.Face ByBrepFace(BrepFace ghBrepFace)
        //{
        //    BrepLoop ghOuterLoop = ghBrepFace.OuterLoop;
        //    BrepLoopList ghLoops = ghBrepFace.Loops;
        //    Topologic.Face face = Topologic.Face.ByExternalInternalBoundaries();
        //}

        /// <summary>
        /// Provides an Icon for the component.
        /// </summary>
        protected override System.Drawing.Bitmap Icon
        {
            get
            {
                //You can add image files to your project resources and access them like this:
                // return Resources.IconForThisComponent;
                return Resources.NMT_borderless_logo_small;
            }
        }

        /// <summary>
        /// Gets the unique ID for this component. Do not change this ID after release.
        /// </summary>
        public override Guid ComponentGuid
        {
            get { return new Guid("6f38afdb-d218-4b22-8b50-853dc46fce9c"); }
        }
    }
}
