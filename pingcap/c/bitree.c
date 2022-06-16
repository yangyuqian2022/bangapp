/*
二叉树Z字形遍历
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct node Node;
typedef struct node
{
    int   val; // Node content
    int   depth; // Depth of current node

    // For Tree
    Node* left; // Left
    Node* right; // right
    // For bi-direction linked list
    Node* parent;
    Node* child;
} Node;

Node* queue;
// 当前已经执行过的打印次数counter
// 1. 如果偶数就左->右
// 2. 如果是奇数右->左
int scanned_layers = 0;
// 左右: head->tail
Node* head;
// 右左: tail->head
Node* tail;

// 根据前面scanned_layers的值来判断，要从head->tail还是tail->head
void cleanup_and_print_queue()
{
    if (scanned_layers % 2 == 0)
    {
        // head -> tail
        // -> find all chilren
        while(head)
        {
            printf("%d\n", head->val);
            // 从队列中移除这个元素
            Node* cur_child = head->child;
            if (cur_child)
            {
                cur_child->parent = NULL;
            }
            // 清空当前数节点上的队列状态
            head->parent = NULL;
            head->child  = NULL;

            head = head->child;
        }
    }
    else
    {
        // tail -> head
        // -> find all parents
        while(tail)
        {
            printf("%d\n", tail->val);
            // 从队列中移除这个元素
            Node* cur_parent = tail->parent;
            if (cur_parent)
            {
                cur_parent->child = NULL;
            }
            
            // 清空当前数节点上的队列状态
            tail->parent = NULL;
            tail->child  = NULL;

            tail = tail->parent;
        }
    }
}

/* Z 形扫描二叉树
 * 1. 先做广度优先遍历
 *    扫描过程中将节点加入一个列表中
 * 2. 如果depth != 列表最后一个节点->depth, 就先将列表中的元素全部清空并依次打印其内容
 *    如果depth == 列表最后一个节点->depth, 就将这个节点加到链表末尾?
 * 3. Z字形打印的实现，全局有一个打印次数counter，每次打印 % 2
 *    如果 == 0就从左往右，
 *    否则就从右往左
 */
void z_scan_tree(Node* root)
{
    // 遍历的根节点为空，直接返回
    if(!root)
    {
        return;
    }

    // 缓存队列空，就初始化队列，当前节点就是队列的根节点
    if (!queue)
    {
        queue = root;
        head = root;
        tail = root;
    }
    else
    {
        // 缓存队列非空，先检查队尾目前的depth与当前节点是否一致，
        // 一致就继续加入队列
        // 否则就打印并清空队列
        // 打印并清空完队列后，再重新将队列替换维当前节点
        if (tail->depth == root->depth)
        {
            // 现在这一层的节点还没扫描完，继续加入队列
            root->child = NULL;
            root->parent = tail;
            tail->child = root;
        } else {
            // 已经开始扫描新的一层，把这一层的节点清空并打印出来
            cleanup_and_print_queue();
        }
    }

    
}

int main(void)
{
    Node* tree = (Node*) malloc(sizeof(Node));
    z_scan_tree(tree);
}