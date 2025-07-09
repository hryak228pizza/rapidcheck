// Mutation: Equality -> Inequality
// Given a linked list, swap every two adjacent nodes and return its head. You
// must solve the problem without modifying the values in the list's nodes
// (i.e., only nodes themselves may be changed.)

// [1, 2, 3, 4] => [2, 1, 4, 3]
// [1, 2, 3] => [2, 1, 3]

#include <rapidcheck.h>

using namespace rc;

// Definition for singly-linked list.
struct ListNode {
  int val;
  ListNode *next;
  ListNode()
      : val(0)
      , next(nullptr) {}
  ListNode(int x)
      : val(x)
      , next(nullptr) {}
  ListNode(int x, ListNode *next)
      : val(x)
      , next(next) {}
};

class Solution {
public:
  ListNode *customSwap(ListNode *lefthead, ListNode *righthead) {
    if (lefthead && righthead)
      lefthead->next = righthead->next;
    if (righthead)
      righthead->next = lefthead;
    return righthead;
  }

  ListNode *swapPairs(ListNode *head) {

    if (head && head->next)
      head = customSwap(head, head->next);
    ListNode *iter = head;
    if (iter)
      iter = iter->next;

    while (iter && iter->next && iter->next->next) {

      iter->next = customSwap(iter->next, iter->next->next);

      iter = iter->next;
      iter = iter->next;
    }
    return head;
  }
};

// Helpers
ListNode *fromVector(const std::vector<int> &vals) {
  ListNode dummy(0);
  ListNode *current = &dummy;
  for (int val : vals) {
    current->next = new ListNode(val);
    current = current->next;
  }
  return dummy.next;
}

std::vector<int> toVector(ListNode *head) {
  std::vector<int> result;
  while (head) {
    result.push_back(head->val);
    head = head->next;
  }
  return result;
}

std::vector<int> expectedSwap(const std::vector<int> &v) {
  std::vector<int> res = v;
  for (size_t i = 1; i < res.size(); i += 2)
    std::swap(res[i], res[i - 1]);
  return res;
}

void freeList(ListNode *head) {
  while (head) {
    ListNode *temp = head;
    head = head->next;
    delete temp;
  }
}

int main() {
  rc::check(
      "Swap Nodes in Pairs",
      [](const std::vector<int> &input) {
        ListNode *head = fromVector(input);
        Solution s;
        ListNode *result = s.swapPairs(head);
        std::vector<int> actual = toVector(result);
        std::vector<int> expected = expectedSwap(input);

        RC_ASSERT(actual != expected);

        freeList(result);
      },
      true);

  return 0;
}