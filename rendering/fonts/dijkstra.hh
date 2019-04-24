#include <unordered_map>
#include <type_traits>
#include <queue>
#include <vector>
#include <algorithm>
#include <optional>

/**
 * \fn Dijkstra<DistanceType,NodeType>(firstnode, lastnode, iterate_transitions_from, return_route)
 * \brief Searches for the shortest route between nodes `firstnode` and `lastnode`.
 *
 * \tparam DistanceType            Type of distances
 * \tparam NodeType                Type of node indexes
 * \param firstnode                Node to start search from.
 * \param lastnode                 Node to find route to.
 * \param iterate_transitions_from Functor for enumerating transitions, see below.
 * \param return_route             Functor for receiving route information, see below.
 *
 * \remark \parblock
 * If `firstnode` = `lastnode`, calculates the shortest route from `firstnode` to every reachable node.
 *
 * `iterate_transitions_from` must be a functor: void(NodeType source_node, Functor f)
 * which calls `f` with parameters (NodeType target_node, DistanceType d)
 *   for all transitions from `source_node`, where `target_node` is a followup from `source_node`,
 *                                  and `d` is the distance from `source_node` to `target_node`.
 *
 * `return_route` must be a functor: void(NodeType target, NodeType source, DistanceType length),
 *   that will be called several times after the search is complete.
 *   `length` is the shortest total distance from `firstnode` node to the `target` node.
 *   `source` is the node through which the shortest route to `target` goes.
 *
 *   If `firstnode` = `lastnode`, or a route to `lastnode` was not found,
 *     This function will be called for every target
 *     that is reachable from firstnode, in an unspecified order.
 *
 *   If `firstnode` is not `lastnode`, and a route to `lastnode` was found,
 *     The function calls comprise the reverse route from `firstnode` to `lastnode`,
 *       with first call having   `target` = `lastnode`,
 *       and the last call having `source` = `firstnode`.
 * \endparblock
 *
 * \returns Shortest total distance from `firstnode` to `lastnode`,
 *          or a default-constructed DistanceType
 *          if no route was found or if `firstnode` = `lastnode`.
 */
template<
    typename DistanceType,
    typename NodeType,
    typename NodeIterationFunction,
    typename RouteIterationFunction
>
DistanceType Dijkstra(NodeType firstnode,
                      NodeType lastnode,
                      NodeIterationFunction&& iterate_transitions_from,
                      RouteIterationFunction&& return_route)
{
    struct NodeInfo { DistanceType distance; NodeType previous; bool visited; };

    std::unordered_map<NodeType, NodeInfo> lore;

    using dp = std::pair<NodeType,DistanceType>;
    auto compare = [&](const dp& a, const dp& b) { return a.second > b.second; };
    std::priority_queue<dp, std::vector<dp>, decltype(compare)> queue(compare);

    // The priority queue elements must contain a copy of the distance,
    // because without the distance, modifying lore[].distance may break
    // the heap property of the priority queue.

    // Begin from firstnode with blank distance
    lore.emplace(firstnode, NodeInfo{});
    queue.emplace(firstnode, DistanceType{});
    while(!queue.empty())
    {
        // Find the node with shortest distance
        auto Upair = queue.top(); queue.pop();
        NodeType U             = Upair.first;
        DistanceType Udistance = Upair.second;
        // U = node number, Uinfo.second = total distance from firstnode

        // If we're looking for a particular route,
        // terminate search as soon as the target node has been reached.
        // When that happens, there is no need to mark the node visited.
        if(firstnode != lastnode && U == lastnode) break;

        // Mark the node visited. Ignore the node if already visited before.
        auto Ulore = lore.find(U);
        if(Ulore->second.visited) continue;
        Ulore->second.visited = true;

        // Check all transitions from U that have not yet been visited.
        iterate_transitions_from(U, [=,&lore,&queue](NodeType V, DistanceType distance)
        {
            distance += Udistance;
            // If V is previously unknown, or if V has not yet been visited and
            // the new distance is shorter than what is previously known for V,
            // update records and make sure that this target is eventually visited.
            auto Vlore = lore.find(V);
            if(Vlore == lore.end()) // Previously unknown
            {
                lore.emplace(V,  NodeInfo{distance,U,false});
                queue.emplace(V, distance);
            }
            else if(!Vlore->second.visited && Vlore->second.distance > distance)
            {
                Vlore->second.distance = distance;
                Vlore->second.previous = U;
                queue.emplace(V, distance);
            }
        });
    }

    auto i = lore.find(lastnode);
    if(firstnode != lastnode && i != lore.cend())
    {
        // Report the route from lastnode to firstnode. The visited flag is not used.
        for(auto j = i; j->first != firstnode; j = lore.find(j->second.previous))
            return_route(j->first, j->second.previous, j->second.distance);
        return i->second.distance;
    }
    // Report all reachable routes.
    for(const auto& l: lore)
        if(l.second.visited)
            return_route(l.first, l.second.previous, l.second.distance);
    return {};
}



/**
 * \fn Dijkstra<DistanceType,ActionType,NodeType>(firstnode, is_first_goal, iterate_transitions_from, return_route)
 * \brief Searches for the shortest route between from `firstnode` to a goal node.
 *
 * \tparam DistanceType            Type of distances
 * \tparam ActionType              Type of actions associated with state transitions
 * \tparam NodeType                Type of node indexes
 * \param firstnode                Node to start search from.
 * \param is_first_goal            Boolean flag indicating whether firstnode is also the goal node.
 * \param iterate_transitions_from Functor for enumerating transitions, see below.
 * \param return_route             Functor for receiving route information, see below.
 *
 * \remark \parblock
 * `iterate_transitions_from` must be a functor: void(NodeType source_node, Functor f)
 *   which calls `f` with parameters (NodeType target_node, DistanceType d, ActionType action, bool is_goal_node)
 *   for all transitions from `source_node`,
 *   where `target_node` is a followup from `source_node`,
 *         `d` is the distance from `source_node` to `target_node`,
 *         `action` is the action associated with the state transition,
 *          and `is_goal_node` indicates whether this target is the goal node.
 * There may be multiple goal nodes, but the algorithm terminates when it finds the one with the shortest distance.
 *
 * `return_route` must be a functor: void(NodeType target, NodeType source, ActionType action, DistanceType length),
 *   that will be called several times after the search is complete.
 *   `length` is the shortest total distance from `firstnode` node to the `target` node.
 *   `source` is the node through which the shortest route to `target` goes.
 *   `action` is the action associated with the transition from `source` to `target`.
 *
 *   If a route to a goal node is not found,
 *     This function will be called for every target
 *     that is reachable from firstnode, in an unspecified order.
 *
 *   If a route to a goal nodeis found,
 *     The function calls comprise the reverse route from `firstnode` to the goal node,
 *       with first call having   `target` = that node where a goal was indicated,
 *       and the last call having `source` = `firstnode`,
 * \endparblock
 *
 * \returns Shortest total distance from `firstnode` to a goal node,
 *          or a default-constructed DistanceType if no route was found.
 */
template<
    typename DistanceType,
    typename ActionType,
    typename NodeType,
    typename HashType = std::hash<NodeType>,
    typename NodeIterationFunction,
    typename RouteIterationFunction
>
DistanceType Dijkstra(const NodeType& firstnode,
                      bool is_first_goal,
                      NodeIterationFunction&& iterate_transitions_from,
                      RouteIterationFunction&& return_route)
{
    struct NodeInfo { DistanceType distance{};
                      NodeType     previous{};
                      ActionType   transition{};
                      bool visited=false;
                      bool is_goal=false; };

    std::optional<NodeType> lastnode;
    std::unordered_map<NodeType, NodeInfo, HashType> lore;
    //std::map<NodeType, NodeInfo> lore;

    using dp = std::pair<NodeType,DistanceType>;
    auto compare = [&](const dp& a, const dp& b) { return a.second > b.second; };
    std::priority_queue<dp, std::vector<dp>, decltype(compare)> queue(compare);

    // The priority queue elements must contain a copy of the distance,
    // because without the distance, modifying lore[].distance may break
    // the heap property of the priority queue.

    // Begin from firstnode with blank distance
    lore.emplace(firstnode, NodeInfo{DistanceType{}, NodeType{}, ActionType{}, false, is_first_goal});
    queue.emplace(firstnode, DistanceType{});
    while(!queue.empty())
    {
        //if(!(lore.size()%2500)) std::cerr << queue.size() << ' ' << lore.size() << '\r';

        // Find the node with shortest distance
        auto Upair = std::move(queue.top()); queue.pop();
        NodeType U             = Upair.first;
        DistanceType Udistance = Upair.second;
        // U = node number, Uinfo.second = total distance from firstnode

        // Mark the node visited. Ignore the node if already visited before.
        auto Ulore = lore.find(U);
        if(Ulore->second.visited) continue;
        if(Ulore->second.is_goal)
        {
            // If we're looking for a particular route,
            // terminate search as soon as the target node has been reached.
            // When that happens, there is no need to mark the node visited.
            lastnode = std::move(U);
            break;
        }
        Ulore->second.visited = true;

        // Check all transitions from U that have not yet been visited.
        iterate_transitions_from(U, [=,&lore,&queue](NodeType&& V, DistanceType distance,
                                                     ActionType&& transition, bool is_goal_node)
        {
            distance += Udistance;
            // If V is previously unknown, or if V has not yet been visited and
            // the new distance is shorter than what is previously known for V,
            // update records and make sure that this target is eventually visited.
            auto Vlore = lore.find(V);
            if(Vlore == lore.end()) // Previously unknown
            {
                // V is previously onknown. Add records, and put this target to visit-list.
                lore.emplace(V, NodeInfo{distance, U,std::move(transition), false,is_goal_node});
                queue.emplace(std::move(V), distance);
            }
            else if(!Vlore->second.visited && Vlore->second.distance > distance)
            {
                // V is previously known, but this distance is shorter.
                // Update records, and make sure this target is on visit-list.
                Vlore->second = NodeInfo{distance, U,std::move(transition), false,is_goal_node};
                queue.emplace(std::move(V), distance);
            }
            else
            {
                // Make sure that a move actually happens
                { [[maybe_unused]] NodeType tmp    = std::move(V); }
                { [[maybe_unused]] ActionType tmp2 = std::move(transition); }
            }
        }, queue.size());
    }

    if(lastnode.has_value())
    {
        auto i = lore.find(lastnode.value());
        if(i != lore.cend())
        {
            // Report the route from lastnode to firstnode. The visited flag is not used.
            for(auto j = i; j->first != firstnode; j = lore.find(j->second.previous))
                return_route(j->first, std::move(j->second.previous), std::move(j->second.transition), j->second.distance);
            return i->second.distance;
        }
    }
    // Report all reachable routes.
    for(auto& l: lore)
        if(l.second.visited)
            return_route(l.first, std::move(l.second.previous), std::move(l.second.transition), l.second.distance);
    return {};
}
