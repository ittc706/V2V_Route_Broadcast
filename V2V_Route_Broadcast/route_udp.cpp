#include<iostream>
#include<iomanip>
#include<fstream>
#include<sstream>
#include"route_udp.h"
#include"config.h"
#include"gtt.h"
#include"wt.h"
#include"vue.h"
#include"vue_physics.h"
#include"function.h"
#include"reflect/context.h"
#include"time_stamp.h"

using namespace std;

int route_udp_route_event::s_event_count = 0;

default_random_engine route_udp::s_engine;

void route_udp_link_event::transimit() {
	//<Warn>:һ������ָ��Ƶ�δ��䣬���ٷֳɶ�����ֱ�ѡ��Ƶ�δ�����

	if (++m_tti_idx == m_tti_num) {
		m_is_finished = true;
	}

	if (get_pattern_idx() < 0 || get_pattern_idx() > 4) throw logic_error("error");
	double sinr = ((wt*)context::get_context()->get_bean("wt"))->calculate_sinr(
		get_source_node_id(),
		get_destination_node_id(),
		get_pattern_idx(),
		route_udp_node::get_node_id_set(get_pattern_idx()));

	if (sinr < ((rrm_config*)context::get_context()->get_bean("rrm_config"))->get_drop_sinr_boundary()){
		m_is_loss = true;
	}
}

int route_udp_node::s_node_count = 0;

default_random_engine route_udp_node::s_engine(time(NULL));

std::vector<std::set<int>> route_udp_node::s_node_id_per_pattern;

const std::set<int>& route_udp_node::get_node_id_set(int t_pattern_idx) {
	return s_node_id_per_pattern[t_pattern_idx];
}

route_udp_node::route_udp_node() {
	context* __context = context::get_context();
	int interval = ((route_config*)__context->get_bean("route_config"))->get_t_interval();
	uniform_int_distribution<int> u_start_broadcast_tti(0, interval);

	m_broadcast_time = u_start_broadcast_tti(s_engine);//��ʼ����һ�η���������Ϣ��ʱ�䣬���ڴ�����
}

pair<int, int> route_udp_node::select_relay_information() {
	pair<int, int> res = make_pair<int, int>(-1, -1);

	//����ѡ·�ɳ���id
	int final_destination_node_id = peek_send_event_queue()->get_final_destination_node_id();
	if (final_destination_node_id != -1) {//�ж��Ƿ�Ϊ�㲥�¼��������������ѡ��һ��
		throw logic_error("error");
	}

	int pattern_num = ((rrm_config*)context::get_context()->get_bean("rrm_config"))->get_pattern_num();

	//�ڵ���δռ�õ�Ƶ���������ѡһ��
	vector<int> candidate;
	for (int pattern_idx = 0; pattern_idx < pattern_num; pattern_idx++) {
			candidate.push_back(pattern_idx);
	}
	if (candidate.size() != 0) {
		uniform_int_distribution<int> u(0, static_cast<int>(candidate.size()) - 1);
		res.second = candidate[u(s_engine)];
	}
	return res;
}

ofstream route_udp::s_logger_link_pdr_distance;
ofstream route_udp::s_logger_delay;

void route_udp::initialize() {
	context* __context = context::get_context();
	int vue_num = get_gtt()->get_vue_num();
	m_node_array = new route_udp_node[vue_num];

	s_logger_link_pdr_distance.open("log/route_udp_link_pdr_distance.txt");
	s_logger_delay.open("log/route_udp_delay.txt");

	route_udp_node::s_node_id_per_pattern = vector<set<int>>(get_rrm_config()->get_pattern_num());
}

void route_udp::process_per_tti() {
	//�¼�����
	event_trigger();

    //����Ҫ��ʼ���͵��¼�
	start_sending_data();

	//���䵱ǰTTI���ڵ��¼�
	transmit_data();
}

void route_udp::event_trigger() {
	context* __context = context::get_context();
	int interval = ((route_config*)__context->get_bean("route_config"))->get_t_interval();

	if (get_time()->get_tti() < ((global_control_config*)__context->get_bean("global_control_config"))->get_ntti()) {
		//�ڳ�ʼ��ʱ����󣬴������ݴ����¼�
		for (int origin_source_node_id = 0; origin_source_node_id < route_udp_node::s_node_count; origin_source_node_id++) {
			route_udp_node& source_node = get_node_array()[origin_source_node_id];
			if (get_time()->get_tti() == source_node.m_broadcast_time) {
				get_node_array()[origin_source_node_id].offer_send_event_queue(
					new route_udp_route_event(origin_source_node_id, -1, get_time()->get_tti(), route_udp_route_event::s_event_count++,1)
				);
				source_node.m_broadcast_time += interval;

				source_node.success_route_event[route_udp_route_event::s_event_count - 1] = 0;//��Ǹý��սڵ��Ѿ��յ������¼��������ظ�����
			}
		}
	}
	

	//route_udp_node& source_node1 = get_node_array()[100];
	//route_udp_node& source_node2 = get_node_array()[101];
	//route_udp_node& source_node3 = get_node_array()[102];
	//if (get_time()->get_tti() == 1) {
	//	get_node_array()[100].offer_send_event_queue(
	//		new route_udp_route_event(100, -1, Broadcast, get_time()->get_tti(), route_udp_route_event::s_event_count++,1)
	//	);

	//	source_node1.success_route_event[route_udp_route_event::s_event_count - 1] = 0;//��Ǹý��սڵ��Ѿ��յ������¼��������ظ�����

	//	log_event(100, -1);
	//	get_node_array()[101].offer_send_event_queue(
	//		new route_udp_route_event(101, -1, Broadcast, get_time()->get_tti(), route_udp_route_event::s_event_count++, 1)
	//	);
	//	log_event(101, -1);

	//	source_node2.success_route_event[route_udp_route_event::s_event_count - 1] = 0;//��Ǹý��սڵ��Ѿ��յ������¼��������ظ�����

	//	get_node_array()[102].offer_send_event_queue(
	//		new route_udp_route_event(102, -1, Broadcast, get_time()->get_tti(), route_udp_route_event::s_event_count++, 1)
	//	);

	//	source_node3.success_route_event[route_udp_route_event::s_event_count - 1] = 0;//��Ǹý��սڵ��Ѿ��յ������¼��������ظ�����

	//	log_event(102, -1);
	//}

	//int temp1 = 0;
	//int temp2 = 0;
	//int temp3 = 0;
	//for (int vuenum = 0; vuenum < vue_physics::get_vue_num(); vuenum++)
	//{
	//	if (vue_physics::get_distance(vuenum, 100) < ((global_control_config*)__context->get_bean("global_control_config"))->get_max_distance()&&vuenum!=100) {
	//		temp1++;
	//	}

	//	if (vue_physics::get_distance(vuenum, 101) < ((global_control_config*)__context->get_bean("global_control_config"))->get_max_distance()&&vuenum!=101) {
	//		temp2++;
	//	}

	//	if (vue_physics::get_distance(vuenum, 102) < ((global_control_config*)__context->get_bean("global_control_config"))->get_max_distance() && vuenum != 102) {
	//		temp3++;
	//	}
	//}
	//cout << "������1��" << temp1 << endl;
	//cout << "������2��" << temp2 << endl;
	//cout << "������3��" << temp3 << endl;
}

void route_udp::start_sending_data() {
	int pattern_num = ((rrm_config*)context::get_context()->get_bean("rrm_config"))->get_pattern_num();

	//����ͬһʱ�̷���Ϣ����������Ϣ��ԭ�����з���Ϣ���¼��ڴ���ǰ��ѡ����Ƶ�β�����ռ��
	for (int source_node_id = 0; source_node_id < route_udp_node::s_node_count; source_node_id++) {
		route_udp_node& source_node = get_node_array()[source_node_id];

		if (source_node.is_send_event_queue_empty()) continue;//��ǰ�����������¼��б�Ϊ�գ���������

		if (source_node.sending_link_event.size() == 0) {//��ǰ�ڵ���һ���¼��Ѿ���ɴ������û��Ҫ������¼�
			//ѡ��Ƶ��
			pair<int, int> select_res = source_node.select_relay_information();

			//<Warn:�����־>
			if (route_udp_node::s_node_id_per_pattern[select_res.second].find(source_node_id) != route_udp_node::s_node_id_per_pattern[select_res.second].end()) throw logic_error("error");
			if (select_res.second < 0 || select_res.second >= pattern_num) throw logic_error("error");

			//ά�������б�
			route_udp_node::s_node_id_per_pattern[select_res.second].insert(source_node_id);

			//�Գ��˸ýڵ�����������ڵ㴴����·�¼�
			for (int dst_id = 0; dst_id < route_udp_node::s_node_count; dst_id++) {

				context *__context = context::get_context();

				if (dst_id == source_node_id || vue_physics::get_distance(source_node.m_send_event_queue.front()->get_origin_source_node_id(), dst_id) >= ((global_control_config*)__context->get_bean("global_control_config"))->get_max_distance()) continue;

				map<int, double>::iterator marked = get_node_array()[dst_id].success_route_event.find(source_node.m_send_event_queue.front()->get_event_id());
				if (marked != get_node_array()[dst_id].success_route_event.end()) continue;//���ĳ�ڵ��Ѿ����չ����¼��򲻽��д��䣨������������

				source_node.sending_link_event.push_back(new route_udp_link_event(
					source_node_id, dst_id, select_res.second, source_node.peek_send_event_queue()->get_tti_num()));

			}
			if (source_node.sending_link_event.size() == 0) {//����㲥���սڵ������ɽ������ٽ����㲥����
				route_udp_route_event* temp = source_node.m_send_event_queue.front();
				source_node.m_send_event_queue.pop();
				delete temp;

				//���Ѿ���������б�ĳ���id��ɾ��
				route_udp_node::s_node_id_per_pattern[select_res.second].erase(source_node_id);
			}
			else {
				m_broadcast_num++;
			}
		}
	}
}

void route_udp::transmit_data() {
	int pattern_num = ((rrm_config*)context::get_context()->get_bean("rrm_config"))->get_pattern_num();

	//������link_event���е�һ�������Ŀ��1�����������¼���Ŀ��2��ά�����սڵ㴫��pattern��״̬
	for (int source_node_id = 0; source_node_id < route_udp_node::s_node_count; source_node_id++) {
		route_udp_node& source_node = get_node_array()[source_node_id];
		if (source_node.sending_link_event.size() == 0) continue;

		//�Ե�ǰ��������link_event���б�������
		vector<route_udp_link_event*>::iterator it;
		for (it = source_node.sending_link_event.begin(); it != source_node.sending_link_event.end(); it++) {

			route_udp_node& destination_node = get_node_array()[(*it)->get_destination_node_id()];

			//�¼�����
			(*it)->transimit();
		}
	}

	//������link_event���еڶ�����������Ѿ�������ϵ��¼����в�����Ŀ��1��ͳ���¼�����ɹ����Ƕ�ʧ��Ŀ��2���޸ķ��ͽڵ�ͽ��սڵ㵱ǰpattern�ϵ�״̬,ά�������б�Ŀ��3������link_event������route_event
	for (int source_node_id = 0; source_node_id < route_udp_node::s_node_count; source_node_id++) {
		route_udp_node& source_node = get_node_array()[source_node_id];
		if (source_node.sending_link_event.size() == 0) continue;

		//�Ե�ǰ��������link_event���б���ά��
		vector<route_udp_link_event*>::iterator it;

		bool all_link_event_finished = false;//�����ж�����link_event�Ƿ�����ϣ���ɾ��link_event
		for (it = source_node.sending_link_event.begin(); it != source_node.sending_link_event.end(); it++) {

			if ((*it)->is_finished()) {
				all_link_event_finished = true;

				int pattern_idx = (*it)->m_pattern_idx;

				route_udp_node& destination_node = get_node_array()[(*it)->get_destination_node_id()];

				int destination_node_id = destination_node.get_id();

				int origin_node_id = source_node.m_send_event_queue.front()->get_origin_source_node_id();

				//����link_event������Ϻ�ά�������б�
				if (it == source_node.sending_link_event.end() - 1) {
					if (route_udp_node::s_node_id_per_pattern[pattern_idx].find(source_node_id) == route_udp_node::s_node_id_per_pattern[pattern_idx].end()) throw logic_error("error");
					if (pattern_idx < 0 || pattern_idx >= pattern_num) throw logic_error("error");
					route_udp_node::s_node_id_per_pattern[pattern_idx].erase(source_node_id);
				}

				//�ж��Ƿ񶪰�
				if ((*it)->get_is_loss()) {
					context *__context = context::get_context();
					if (vue_physics::get_distance(origin_node_id, destination_node_id) < ((global_control_config*)__context->get_bean("global_control_config"))->get_max_distance()) {
						map<int, double>::iterator marked = destination_node.failed_route_event.find(source_node.m_send_event_queue.front()->get_event_id());
						map<int, double>::iterator _marked = destination_node.success_route_event.find(source_node.m_send_event_queue.front()->get_event_id());
						if (marked == destination_node.failed_route_event.end() && _marked == destination_node.success_route_event.end()) {//������¼�û�б����գ��������
							destination_node.failed_route_event[source_node.m_send_event_queue.front()->get_event_id()] = vue_physics::get_distance(origin_node_id, destination_node_id);//��Ǹý��սڵ��Ѿ��յ������¼��������ظ�����
							m_failed_route_event_num++;
						}
					}
					if (source_node.m_send_event_queue.empty()) throw logic_error("error");

					//����link_event������Ϻ�ɾ��route_event
					if (it == source_node.sending_link_event.end() - 1) {
						route_udp_route_event* temp = source_node.m_send_event_queue.front();

						//ɾ��route_event
						source_node.m_send_event_queue.pop();

						delete temp;
					}
				}
				else {
					//����ǹ㲥�������·���㷨���д���
					map<int, double>::iterator marked = destination_node.success_route_event.find(source_node.m_send_event_queue.front()->get_event_id());
					if (marked == destination_node.success_route_event.end()) {//������¼�û�б����գ��������

						destination_node.success_route_event[source_node.m_send_event_queue.front()->get_event_id()] = vue_physics::get_distance(origin_node_id, destination_node_id);//��Ǹý��սڵ��Ѿ��յ������¼��������ظ�����
						m_success_route_event_num++;

						s_logger_link_pdr_distance << source_node.m_send_event_queue.front()->m_hop << "," << get_gtt()->get_vue_array()[destination_node.get_id()].get_physics_level()->m_absx << "," << get_gtt()->get_vue_array()[destination_node.get_id()].get_physics_level()->m_absy << endl;

						if (source_node.m_send_event_queue.front()->m_hop != 0) {
							destination_node.offer_send_event_queue(
								new route_udp_route_event(origin_node_id, -1,get_time()->get_tti(), source_node.m_send_event_queue.front()->get_event_id(), source_node.m_send_event_queue.front()->m_hop - 1)
							);//�����Ҫ�����㲥���ڽ��սڵ㷢�Ͷ���������¼�


						}
						map<int, double>::iterator failed = destination_node.failed_route_event.find(source_node.m_send_event_queue.front()->get_event_id());
						if (failed != destination_node.failed_route_event.end()) {
							m_failed_route_event_num--;
							destination_node.failed_route_event.erase(failed);
						}
					}

					if (source_node.m_send_event_queue.empty()) throw logic_error("error");

					//����link_event������Ϻ�ɾ��route_event
					if (it == source_node.sending_link_event.end() - 1) {
						route_udp_route_event* temp = source_node.m_send_event_queue.front();
						source_node.m_send_event_queue.pop();
						delete temp;
					}
				}
			}
		}

		//������ɵ�link_event ������Ϻ�ɾ������link_event
		if (all_link_event_finished == true) {
			vector<route_udp_link_event*>::iterator it = source_node.sending_link_event.begin();
			while (it != source_node.sending_link_event.end()) {
				delete *it;
				it++;
			}
			source_node.sending_link_event.clear();
		}
	}
}

//�򵥵ĸ��ݾ���ά���ڽӱ�
void route_udp::update_route_table_from_physics_level() {

}
