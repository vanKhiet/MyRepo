
#include "Structs/FmGraph.h"

FmGraph::FmGraph()
{
}

FmGraph::FmGraph(const TArray<FmEdge>& EdgesPara)
{

	for (const auto& Edge : EdgesPara)
	{
		if (!Edges.Contains(Edge))
			Edges.Add(Edge);
		if (!Vertices.Contains(Edge.Vertex1))
			Vertices.Add(Edge.Vertex1);
		if (!Vertices.Contains(Edge.Vertex2))
			Vertices.Add(Edge.Vertex2);
	}
}


void FmGraph::DrawGraph(UWorld* World, FColor Color, float Thickness)
{
	for (auto& Vertex : Vertices)
	{
		DrawDebugSphere(World, Vertex, 10.f, 15, Color, true, -1.f, 0U, Thickness);
	}
	for (auto& Edge : Edges)
	{
		DrawDebugLine(World, Edge.Vertices[0], Edge.Vertices[1], Color, true, -1.f, 0U, Thickness);
	}
}

struct GraphPoint;
struct GraphEdge;
struct Subset;

struct GraphPoint
{
	FVector Location;
	GraphPoint(FVector InLocation) { Location = InLocation; };
	Subset* ParentPointer = nullptr;
	bool operator==(const GraphPoint& Other) const { return Location == Other.Location; };
};


struct GraphEdge
{
	TArray<GraphPoint> Points = {};
	TArray<GraphPoint*> RealPoints = { nullptr, nullptr };
	float Length;
	GraphEdge(GraphPoint InPoint1, GraphPoint InPoint2) { Points.Add(InPoint1); Points.Add(InPoint2); Length = FVector::Distance(InPoint1.Location, InPoint2.Location); }
	bool operator==(const GraphEdge& Other) const { return (RealPoints == Other.RealPoints); }
};

struct Subset
{
	TArray<GraphEdge> Edges = {};
	Subset(const GraphEdge& InEdge) { Edges.Add(InEdge); }

	bool operator==(const Subset& Other) const { return Edges == Other.Edges; }
};

FmGraph FmGraph::CreateMST(UWorld* World, int InEdgeIndexToRun)
{
	TArray <Subset> ParentSet;
	TArray<GraphPoint> GraphPoints = {};
	TArray<GraphEdge> GraphEdges = {};

	// Add all the edges in edges to begin edge array
	for (const FmEdge& Edge : Edges)
	{
		GraphEdges.Add(GraphEdge( GraphPoint(Edge.Vertex1), GraphPoint(Edge.Vertex2) ));
	}

	// Add all the points in vertices to point list
	for (const FVector& Vertex : Vertices)
	{
		GraphPoints.Add(GraphPoint(Vertex));
	}

	// Set all the real point's pointers
	for (GraphEdge& Edge : GraphEdges)
	{
		for (int PointInEdgeIndex = 0; PointInEdgeIndex < 2; PointInEdgeIndex++)
		{
			for (int PointInListIndex = 0; PointInListIndex < GraphPoints.Num(); PointInListIndex++)
			{
				if (Edge.Points[PointInEdgeIndex] == GraphPoints[PointInListIndex])
				{
					Edge.RealPoints[PointInEdgeIndex] = &GraphPoints[PointInListIndex];
					break;
				}
			}
		}
	}

	// Set all the edge's point to empty array
	for (GraphEdge& Edge : GraphEdges)
	{
		Edge.Points.Empty();
	}

	GraphEdges.Sort([](const GraphEdge& A, const GraphEdge& B) { return A.Length < B.Length; }); 
	UE_LOG(LogTemp, Warning, TEXT("Number of Edges: %d"), Edges.Num());
	UE_LOG(LogTemp, Warning, TEXT("Number of Begin Edges: %d"), GraphEdges.Num());


	// Start of the big loop
	for (int i = 0; i < InEdgeIndexToRun; i++)
	{
		const GraphEdge& CurrentEdge = GraphEdges[i];

		Subset*& Point0Parent = CurrentEdge.RealPoints[0]->ParentPointer;
		Subset*& Point1Parent = CurrentEdge.RealPoints[1]->ParentPointer;

		// First case: If both point in edge have no parent
		if (Point0Parent == nullptr && Point1Parent == nullptr) 
		{
			Subset NewSubset = Subset(CurrentEdge);
			ParentSet.Add(NewSubset);
			for (int j = 0; j < ParentSet.Num(); j++)
			{
				if (ParentSet[j] == NewSubset)
				{
					Point0Parent = &ParentSet[j];
					Point1Parent = &ParentSet[j];
					break;
				}
			}
			DrawDebugLine(World, CurrentEdge.RealPoints[0]->Location, CurrentEdge.RealPoints[1]->Location, FColor::Green, true, -1.f, 0U, 15.f);
			continue;
		}

		// Second case: If the first point have parent and the second doesn't
		else if (Point0Parent != nullptr && Point1Parent == nullptr)
		{
			Point0Parent->Edges.Add(CurrentEdge);
			Point1Parent = Point0Parent;
			DrawDebugLine(World, CurrentEdge.RealPoints[0]->Location, CurrentEdge.RealPoints[1]->Location, FColor::Green, true, -1.f, 0U, 15.f);
			continue;
		}

		// Third case: If the first point doesn't have parent and the second does
		else if (Point0Parent == nullptr && Point1Parent != nullptr)
		{
			Point1Parent->Edges.Add(CurrentEdge);
			Point0Parent = Point1Parent;
			DrawDebugLine(World, CurrentEdge.RealPoints[0]->Location, CurrentEdge.RealPoints[1]->Location, FColor::Green, true, -1.f, 0U, 15.f);
			continue;
		}

		// Fourth case: If both point have parent
		else if (Point0Parent != nullptr && Point1Parent != nullptr)
		{
			// If they have the same parent
			if (Point0Parent == Point1Parent)
			{
				DrawDebugLine(World, CurrentEdge.RealPoints[0]->Location, CurrentEdge.RealPoints[1]->Location, FColor::Red, true, -1.f, 0U, 15.f);
				continue;
			}
			// If they have different parent
			else
			{
				// Create and add a brand new subset
				Subset NewSubset = Subset(CurrentEdge);
				NewSubset.Edges.Append(Point0Parent->Edges);
				NewSubset.Edges.Append(Point1Parent->Edges);
				ParentSet.Add(NewSubset);

				// Remove the old subsets
				ParentSet.Remove(*Point0Parent);
				ParentSet.Remove(*Point1Parent);
				for (int j = 0; j < ParentSet.Num(); j++)
				{
					if (ParentSet[j] == NewSubset)
					{
						Point0Parent = &ParentSet[j];
						Point1Parent = &ParentSet[j];
						break;
					}
				}

				DrawDebugLine(World, CurrentEdge.RealPoints[0]->Location, CurrentEdge.RealPoints[1]->Location, FColor::Green, true, -1.f, 0U, 15.f);
				continue;
			}
		}
	}

	// Draw debug string at each point's location
	for (const GraphPoint& Point : GraphPoints)
	{
		for (int SubsetIndex = 0; SubsetIndex < ParentSet.Num(); SubsetIndex++)
		{
			if (Point.ParentPointer == &ParentSet[SubsetIndex])
				DrawDebugString(World, Point.Location, FString::FromInt(SubsetIndex), nullptr, FColor::Yellow);
		}
	}

	int NumberOfSubset = ParentSet.Num();
	UE_LOG(LogTemp, Warning, TEXT("Number of subset: %d"), NumberOfSubset);

	return FmGraph();
}
