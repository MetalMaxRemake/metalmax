"""Translate the reviewed script AST into the supported remake operations."""
OP_NAMES = ['Stop', 'Text', 'EventText', 'SetEvent', 'ClearEvent', 'IfEvent',
            'IfPosition', 'IfYes', 'IfTeam', 'IfDead', 'IfLevel', 'IfMoney',
            'IfItem', 'IfTreasure', 'Jump', 'Loop', 'Wait', 'WaitSet', 'WaitClear',
            'Face', 'Move', 'MoveTo', 'Teleport', 'Wander', 'Chase', 'Animate',
            'Hide', 'Recover', 'Spend', 'Recruit', 'Battle', 'ChangeModel', 'SetFrame',
            'FacePlayer','FaceBack','Offset','IfArea','IfFace','SetAttrs','SpriteBank',
            'ModelDelta','Unsupported']

def compile_script(ast, types, unsupported):
    nodes=[];labels={};fixups=[]
    def emit(op,a=0,b=0,c=0,d=0,target=-1):nodes.append([types.index(op),a,b,c,d,target]);return len(nodes)-1
    def block(seq):
        for n in seq:
            if n.get('label'):labels[n['label']]=len(nodes)
            kind=n['$type'];op=n.get('op','');kids=n.get('scripts',[])
            cond={'Event':('IfEvent','eventCode'),'Xy':('IfPosition','x'),'Yes':('IfYes',None),'Team':('IfTeam','player'),'TeammateDead':('IfDead','teammate'),'Level':('IfLevel','level'),'Money':('IfMoney','money'),'HasItem':('IfItem','item'),'Treasure':('IfTreasure','treasure')}
            if kind=='Event' and 'trueLine' in n:emit('EventText',n['eventCode'],n['page'],n['trueLine'],n['falseLine']);continue
            if kind=='Event' and op=='62':emit('WaitClear',n['eventCode']);continue
            if kind in cond and ('isLoopList' in n or 'scripts' in n):
                name,key=cond[kind];idx=emit(name,n.get(key,0) if key else 0,n.get('y',0),int(n.get('isLoopList',False)))
                if n.get('targetLabel'):fixups.append((idx,n['targetLabel']))
                else:
                    block(kids)
                    if n.get('isLoopList'):emit('Jump',target=idx)
                    nodes[idx][5]=len(nodes)
                continue
            if kind=='LoopScript':
                begin=len(nodes);block(kids);emit('Loop',n.get('count',0),target=begin);continue
            if kind=='JmpScript':idx=emit('Jump');fixups.append((idx,n['targetLabel']));continue
            if kind=='Quote':emit('Text',n['page'],n['line']);continue
            if kind=='EndScript':emit('Stop');continue
            if kind in ('Open','Close'):emit('SetEvent' if kind=='Open' else 'ClearEvent',n['eventCode']);continue
            if kind=='Time':emit('Wait',n['time']);continue
            if kind=='Wait':emit('WaitSet',n['eventCode']);continue
            if kind in ('Up','Down','Left','Right'):
                if op in ('0C','0D','0E','0F'):emit('Face', ['Up','Down','Left','Right'].index(kind))
                elif op in ('12','13','14','15'):emit('Move',['Up','Down','Left','Right'].index(kind),n.get('count',1))
                else:unsupported[kind+':'+op]+=1;emit('Unsupported',int(op or 'ff',16))
                continue
            if kind=='To':emit('MoveTo',n['x'],n['y']);continue
            if kind=='Tp':emit('Teleport',n['x'],n['y']);continue
            if kind=='Wander':emit('Wander',n.get('x1',-1),n.get('y1',-1),n.get('x2',-1),n.get('y2',-1));continue
            if kind=='Chase':emit('Chase');continue
            if kind=='Act':emit('Animate');continue
            if kind in ('TileType','Set'):emit('ChangeModel',n['model'],int(kind=='Set'));continue
            if kind=='Frame':emit('SetFrame',n['frame']);continue
            if kind=='Attrs':emit('SetAttrs',n['attrs']);continue
            if kind=='Sprite':emit('SpriteBank',n['bank']);continue
            if kind=='Next':emit('ModelDelta',1);continue
            if kind=='Previous':emit('ModelDelta',-1);continue
            if kind=='Resume':emit('Animate');continue
            # TpOffset is screen-relative. Keep it unsupported until its
            # camera semantics are implemented, rather than invent a delta.
            if kind=='Player' and op=='11':emit('FacePlayer');continue
            if kind=='Back' and op=='10':emit('FaceBack');continue
            if kind=='HideScript':emit('Hide');continue
            if kind=='RemoveScript':emit('Stop');continue
            if kind=='RecoverScript':emit('Recover',n.get('player',0));continue
            if kind=='Spend':emit('Spend',n['money']);continue
            if kind=='Now':emit('Recruit',n['player']);continue
            if kind=='BattleScript':emit('Battle',n['monster'],n.get('eventCode',0),n.get('stage',0));continue
            if kind=='SpeakerScript':continue
            if kind in ('RideTank','Riding','Here') and ('isLoopList' in n or 'scripts' in n):
                # There is no tank state yet; these predicates are false.
                # Skip their blocks without inventing a riding transition.
                idx=emit('Jump');block(kids);nodes[idx][5]=len(nodes);continue
            # Unsupported effects are a barrier: never execute later story
            # side effects after silently skipping an unknown instruction.
            unsupported[kind+':'+op]+=1;emit('Unsupported',int(op or 'ff',16))
    block(ast);emit('Stop')
    for idx,label in fixups:
        if label in labels:nodes[idx][5]=labels[label]
        else:nodes[idx][0]=types.index('Unsupported')
    return nodes
