#pragma once
#include<iostream>
#include<vector>
#include<list>
#include<queue>
#include<set>
#include<map>
#include<utility>
#include<random>
#include<string>
#include"route.h"
#include"config.h"
#include"reflect/object.h"
#include"reflect/context.h"

using namespace std;

enum node_type {
	VUE,
	RSU
};

/*
* ����·�ɲ㣬���漰����������������Ϊnode
* ����һ���ڵ㣬�շ�ì�ܡ���ͬһʱ��ֻ���գ�����ֻ�ܷ�
* ��һ���ڵ㴦���շ�״̬ʱ���ؾ�һ������
* ���ڵ㴦����״̬ʱ���ýڵ����ΪԴ�ڵ����ϢҲ��������Ϻ��ٽ��з��ͣ�������ת����Ϣ֮��
* ���ڵ㴦�ڿ���״̬����ͬʱ�յ���������ת���������Ӧ��һ�����ܾ�����(������չΪ���ȼ�Ӧ��)
*/

class route_udp_node;

class route_udp_route_event {
	/*
	* �¼����ڵ���ʼʱ��
	*/
private:
	int m_start_tti = 0;

public:
	int get_start_tti() {
		return m_start_tti;
	}

public:
	static int s_event_count;

	/*
	* Դ�ڵ�
	*/
private:
	const int m_origin_source_node_id;
public:
	int get_origin_source_node_id() {
		return m_origin_source_node_id;
	}

	/*
	* Ŀ�Ľڵ�,�㲥ʱĿ�Ľڵ�Ϊ-1
	*/
private:
	const int m_final_destination_node_id;
public:
	int get_final_destination_node_id() {
		return m_final_destination_node_id;
	}

	/*
	* ��ĿǰΪֹ������
	*/
public:
	int m_hop = 0;

	/*
	* �¼�id
	*/
private:
	const int m_event_id;
public:
	int get_event_id() { return m_event_id; }

	/*
	* ���ݰ���С��ӳ���TTI
	*/
private:
	const int m_tti_num;
public:
	int get_tti_num() {
		return m_tti_num;
	}

public:
	/*
	* ���캯�����ṩ���¼�����ģ�����
	*/
	route_udp_route_event(int t_source_node, int t_destination_node, int current_tti, int event_id, int hop) :
		m_event_id(event_id),
		m_origin_source_node_id(t_source_node),
		m_final_destination_node_id(t_destination_node),
		m_hop(hop),
		m_start_tti(current_tti),
		m_tti_num(((tmc_config*)context::get_context()->get_bean("tmc_config"))->get_package_num()) {
	}

};

class route_udp_link_event {
	friend class route_udp_node;
	friend class route_udp;

private:
	/*
	* ��ǰ��·Դ�ڵ�id
	*/
	const int m_source_node_id;
public:
	int get_source_node_id() {
		return m_source_node_id;
	}

	/*
	* ��ǰ��·Ŀ�Ľڵ�id
	*/
private:
	const int m_destination_node_id;
public:
	int get_destination_node_id() {
		return m_destination_node_id;
	}

	/*
	* ���ݰ�����
	* ��Ҫ���͵����ݷ�װ��IP���ݰ�����������ЩIP���ݰ���ʧ����һ������ô�������ݰ����㶪ʧ
	*/
private:
	const int m_tti_num;


	/*
	* ռ�õ�pattern���
	*/
private:
	int m_pattern_idx;
	void set_pattern_idx(int t_pattern_idx) {
		m_pattern_idx = t_pattern_idx;
	}
public:
	int get_pattern_idx() {
		return m_pattern_idx;
	}

	/*
	* ��Ǳ�����ǰʱ�̴��䵽�׼���TTI
	*/
private:
	int m_tti_idx = 0;
public:
	int get_tti_idx() { return m_tti_idx; }

	/*
	* ��Ǳ����Ƿ������(�����Ƿ�������)
	*/
private:
	bool m_is_finished = false;
public:
	bool is_finished() { return m_is_finished; }

	/*
	* ��Ǳ����Ƿ�������
	*/
private:
	bool m_is_loss = false;
public:
	bool get_is_loss() { return m_is_loss; }

public:
	route_udp_link_event(int t_source_node_id, int t_destination_node_id, int t_pattern_idx, int t_package_num) :
		m_source_node_id(t_source_node_id),
		m_destination_node_id(t_destination_node_id),
		m_pattern_idx(t_pattern_idx),
		m_tti_num(t_package_num) {}

	/*
	* �������ݰ��ķ���
	*/
	void transimit();
};

class route_udp_node {
	friend class route_udp;
public:
	map<int, double> success_route_event;
	map<int, double> failed_route_event;
public:
	int m_broadcast_time;//�´ι㲥��ʱ��
public:
	node_type s_node_type;
private:
	/*
	* ���ڷ��͵�link_eventָ�룬ÿ�����ŵ���һ��
	*/
	std::vector<route_udp_link_event*> sending_link_event;

	/*
	* �ڵ���
	*/
	static int s_node_count;

	/*
	* ���������
	*/
	static std::default_random_engine s_engine;

	/*
	* ���ڷ���(ǿ��һ��:��״̬�Ľڵ�)��node�ڵ�
	* ����±�Ϊpattern���
	*/
	static std::vector<std::set<int>> s_node_id_per_pattern;
public:
	static const std::set<int>& get_node_id_set(int t_pattern_idx);

	/*
	* ��ǰ�ڵ�����ͳ�������
	*/
private:
	std::queue<route_udp_route_event*> m_send_event_queue;
public:
	void offer_send_event_queue(route_udp_route_event* t_event) {
		m_send_event_queue.push(t_event);
	}
	route_udp_route_event* poll_send_event_queue() {
		route_udp_route_event* temp = m_send_event_queue.front();
		m_send_event_queue.pop();
		return temp;
	}
	route_udp_route_event* peek_send_event_queue() {
		return m_send_event_queue.front();
	}
	bool is_send_event_queue_empty() {
		return m_send_event_queue.empty();
	}

private:
	/*
	* �ڵ�id
	*/
	const int m_id = s_node_count++;
public:
	int get_id() {
		return m_id;
	}

public:
	/*
	* ���캯��
	*/
	route_udp_node();

public:
	/*
	* ѡ������ת���ĳ����Լ���Ӧ��Ƶ��
	* first�ֶ�Ϊ����id
	* second�ֶ�ΪƵ�α��
	* ����һ���ֶ�Ϊ-1���ʹ���ѡ��ʧ��
	*/
	std::pair<int, int> select_relay_information();
};

class v2x_time;
class gtt;
class wt;
class rrm_config;
class tmc_config;
class route_config;

class route_udp :public route {
	REGISTE_MEMBER_HEAD(route_udp)
public:
	int s_car_num;//��������
	int s_rsu_num;//·�ߵ�Ԫ����
private:
	/*
	* ���������
	*/
	static std::default_random_engine s_engine;

	/*
	* ��־�����
	*/
	static std::ofstream s_logger_link_pdr_distance;
	static std::ofstream s_logger_delay;

private:
	/*
	* �ڵ�����
	*/
	route_udp_node* m_node_array;
public:
	route_udp_node* get_node_array() {
		return m_node_array;
	}

private:

	/*
	* �ɹ�/ʧ�ܴ�����¼�����
	*/
	int m_success_route_event_num = 0;

	int m_failed_route_event_num = 0;

	int m_broadcast_num = 0;

	int m_event_num = 0;
public:

	int get_success_route_event_num(){
		return m_success_route_event_num;
	}
	int get_failed_route_event_num() {
		return m_failed_route_event_num;
	}
	int get_broadcast_num() {
		return m_broadcast_num;
	}
	int get_event_num() {
		return m_event_num;
	}

private:
	v2x_time* m_time;
	gtt* m_gtt;
	wt* m_wt;
	rrm_config* m_rrm_config;
	tmc_config* m_tmc_config;
	route_config* m_route_config;

	void set_time(object* t_time) {
		m_time = (v2x_time*)t_time;
	}
	void set_gtt(object* t_gtt) {
		m_gtt = (gtt*)t_gtt;
	}
	void set_wt(object* t_wt) {
		m_wt = (wt*)t_wt;
	}
	void set_rrm_config(object* t_rrm_config) {
		m_rrm_config = (rrm_config*)t_rrm_config;
	}
	void set_tmc_config(object* t_tmc_config) {
		m_tmc_config = (tmc_config*)t_tmc_config;
	}
	void set_route_config(object* t_route_config) {
		m_route_config = (route_config*)t_route_config;
	}
public:
	v2x_time* get_time() override {
		return m_time;
	}

	gtt* get_gtt() override {
		return m_gtt;
	}

	wt* get_wt() override {
		return m_wt;
	}

	rrm_config* get_rrm_config() override {
		return m_rrm_config;
	}

	tmc_config* get_tmc_config() override {
		return m_tmc_config;
	}

	route_config* get_route_config() override {
		return m_route_config;
	}

	void initialize() override;

	void process_per_tti() override;

	void update_route_table_from_physics_level() override;

private:
	/*
	* ��������¼�
	*/
	void event_trigger();

	/*
	* ����Ҫ��ʼ���͵��¼�
	*/
	void start_sending_data();

	/*
	* ���䵱ǰTTI���ڵ��¼�
	*/
	void transmit_data();

};
