local game_status = {
    Running = 0,
    GameOver = 1
}

local board_w = 10
local board_h = 16
local status = nil

local board = {}
local piece = {}
local next_piece = {}

local score = 0
local move_delay = 0
local gravity_timer = 0

local score = 0
local level = 1
local base_gravity = 40
local min_gravity = 5

local function create_board(w, h)
    local result = {}
    for y = 1, h do
        result[y] = {}
        for x = 1, w do
            result[y][x] = 0
        end
    end
    return result
end

local bag = {"I", "O", "T", "S", "Z", "J", "L"}
local function create_piece()
    local shapes = {
        I = {{{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
             {{0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}}},
        O = {{{1, 1}, {1, 1}}},
        T = {{{0, 1, 0}, {1, 1, 1}, {0, 0, 0}}, {{0, 1, 0}, {0, 1, 1}, {0, 1, 0}}, {{0, 0, 0}, {1, 1, 1}, {0, 1, 0}},
             {{0, 1, 0}, {1, 1, 0}, {0, 1, 0}}},
        S = {{{0, 1, 1}, {1, 1, 0}, {0, 0, 0}}, {{1, 0, 0}, {1, 1, 0}, {0, 1, 0}}},
        Z = {{{1, 1, 0}, {0, 1, 1}, {0, 0, 0}}, {{0, 1, 0}, {1, 1, 0}, {1, 0, 0}}},
        J = {{{0, 1, 0}, {0, 1, 0}, {1, 1, 0}}, {{1, 0, 0}, {1, 1, 1}, {0, 0, 0}}, {{0, 1, 1}, {0, 1, 0}, {0, 1, 0}},
             {{0, 0, 0}, {1, 1, 1}, {0, 0, 1}}},
        L = {{{0, 1, 0}, {0, 1, 0}, {0, 1, 1}}, {{0, 0, 0}, {1, 1, 1}, {1, 0, 0}}, {{1, 1, 0}, {0, 1, 0}, {0, 1, 0}},
             {{0, 0, 1}, {1, 1, 1}, {0, 0, 0}}}
    }

    if #bag == 0 or bag == nil then
        bag = {"I", "O", "T", "S", "Z", "J", "L"}
        for i = #bag, 2, -1 do
            local j = math.random(i)
            bag[i], bag[j] = bag[j], bag[i]
        end
    end

    local shape_key = table.remove(bag)

    return {
        shape = shapes[shape_key],
        rotation = 1,
        x = 4,
        y = 0,
        style = math.random(1, 6)
    }
end

local function update_level()
    level = math.floor(score / 10) + 1
end

local function gravity_delay()
    return math.max(min_gravity, base_gravity - (level - 1) * 2)
end

local function add_score(lines)
    local base = {1, 3, 5, 8}
    if lines >= 1 then
        lines = math.min(lines, 4)
        score = score + base[lines] * level
    end
end

local function steriliza_rotation(rotation)
    return ((rotation - 1) % #piece.shape) + 1
end

local function is_collide(x, y, rotation)
    local rot = steriliza_rotation(rotation)
    local shape = piece.shape[rot]

    for dy = 1, #shape do
        for dx = 1, #shape[dy] do
            if shape[dy][dx] ~= 0 then
                local board_x = x + dx - 1
                local board_y = y + dy - 1
                if board_x < 1 or board_x > board_w or board_y > board_h then
                    return true
                end
                if board_y >= 1 and board[board_y][board_x] ~= 0 then
                    return true
                end
            end
        end
    end

    return false
end

local function place_piece()
    local rot = steriliza_rotation(piece.rotation)
    local shape = piece.shape[rot]

    for dy = 1, #shape do
        for dx = 1, #shape[dy] do
            if shape[dy][dx] ~= 0 then
                local board_x = piece.x + dx - 1
                local board_y = piece.y + dy - 1
                if board_x >= 1 and board_x <= board_w and board_y >= 1 and board_y <= board_h then
                    board[board_y][board_x] = piece.style
                end
            end
        end
    end
end

local function clear_lines()
    local y = board_h
    local lines = 0
    while y >= 1 do
        local full = true
        for x = 1, board_w do
            if board[y][x] == 0 then
                full = false
                break
            end
        end
        if full then
            for k = y, 2, -1 do
                board[k] = board[k - 1]
            end
            board[1] = {}
            for x = 1, board_w do
                board[1][x] = 0
            end
            lines = lines + 1
        else
            y = y - 1
        end
    end
    update_level()
    add_score(lines)
end

local function draw_board()
    for y = 1, board_h do
        for x = 1, board_w do
            if board[y][x] ~= 0 then
                spr(board[y][x], (x - 1) * 8, (y - 1) * 8)
            end
        end
    end
end

local function draw_piece()
    local rot = steriliza_rotation(piece.rotation)
    local shape = piece.shape[rot]
    for dy = 1, #shape do
        for dx = 1, #shape[dy] do
            if shape[dy][dx] ~= 0 then
                local board_x = piece.x + dx - 1
                local board_y = piece.y + dy - 1
                if board_x >= 1 and board_x <= board_w and board_y >= 1 and board_y <= board_h then
                    spr(piece.style, (board_x - 1) * 8, (board_y - 1) * 8)
                end
            end
        end
    end
end

local function draw_next_piece()
    local shape = next_piece.shape[next_piece.rotation]
    for dy = 1, #shape do
        for dx = 1, #shape[dy] do
            if shape[dy][dx] ~= 0 then
                spr(next_piece.style, (10 + dx) * 8, dy * 8)
            end
        end
    end
end

function reset_game()
    piece = create_piece()
    next_piece = create_piece()
    board = create_board(board_w, board_h)
    status = game_status.Running
    score = 1
    score_add = 1
end

function init()
    reset_game()
    palt(0, true)
end

function draw()
    rect(80, 0, 96, 128, 2, true)
    line(80, 0, 80, 128, 15)
    draw_board()
    draw_piece()
    draw_next_piece()
    print("\x80\x81\x82\x83\x84", 88, 40, 3, 4, 7)
    print(tostring(score), 88, 51)
end

function update()
    if status ~= game_status.Running then
        return
    end

    gravity_timer = gravity_timer + 1

    if gravity_timer >= gravity_delay() then
        gravity_timer = 0

        if not is_collide(piece.x, piece.y + 1, piece.rotation) then
            piece.y = piece.y + 1
        else
            place_piece()
            clear_lines()
            piece = next_piece
            next_piece = create_piece()
            if is_collide(piece.x, piece.y, piece.rotation) then
                piece.y = -10
                status = game_status.GameOver
            end
        end
    end

    if keyp(82) then
        local r = steriliza_rotation(piece.rotation + 1)
        if not is_collide(piece.x, piece.y, r) then
            piece.rotation = r
        end
    elseif keyp(80) then
        if not is_collide(piece.x - 1, piece.y, piece.rotation) then
            piece.x = piece.x - 1
        end
    elseif keyp(79) then
        if not is_collide(piece.x + 1, piece.y, piece.rotation) then
            piece.x = piece.x + 1
        end
    elseif key(81) then
        if move_delay <= 0 then
            if not is_collide(piece.x, piece.y + 1, piece.rotation) then
                piece.y = piece.y + 1
                move_delay = 2
                gravity_timer = 0
            end
        end
    end

    move_delay = move_delay - 1
end
